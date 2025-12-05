#include "VideoOpenGLWidget.h"
#include<QDebug>
#include<QPainter>
#include<QDateTime>
//Shader代码
//顶点着色器：传递顶点坐标和纹理坐标
static const char* VERTEX_SHADER_SOURCE = R"(
	attribute vec4 vertexIn;//顶点坐标
	attribute vec2 textureIn;//纹理坐标
	varying vec2 textureOut;//输出变量，传递给片元着色器
	void main(void)//每个顶点调用一次
	{
		gl_Position = vertexIn;//对于顶点坐标不做变换
		textureOut = textureIn;//纹理坐标传递给片元着色器
	}
)";

//片段着色器：将YUV转换为RGB
static const char* FRAGMENT_SHADER_SOURCE = R"(
	varying vec2 textureOut;//接收顶点着色器的输出变量
	uniform sampler2D tex_y;
	uniform sampler2D tex_u;
	uniform sampler2D tex_v;
	void main(void){
		vec3 yuv;
		vec3 rgb;
		yuv.x=texture2D(tex_y,textureOut).r;
		yuv.y=texture2D(tex_u,textureOut).r-0.5;
		yuv.z=texture2D(tex_v,textureOut).r-0.5;
		rgb=mat3(1.0, 1.0, 1.0,
				0.0, -0.39465, 2.03211,
				1.13983, -0.58060, 0.0)*yuv;
		gl_FragColor=vec4(rgb,1.0);
	}
)";

VideoOpenGLWidget::VideoOpenGLWidget(QWidget* parent) 
	:QOpenGLWidget(parent)
{
}

VideoOpenGLWidget::~VideoOpenGLWidget() {
	if (m_vbo) {
		m_vbo->destroy();
		delete m_vbo;
	}
	if (m_texturesCreated) {
		glDeleteTextures(3, m_textureIds);
	}
	if (m_program) {
		delete m_program;
	}
	doneCurrent();
}

void VideoOpenGLWidget::initTextures() {
	//生成3个纹理ID
	glGenTextures(3, m_textureIds);

	//预设纹理参数
	for (int i = 0; i < 3; i++) {
		glBindTexture(GL_TEXTURE_2D, m_textureIds[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	m_texturesCreated = true;
}

void VideoOpenGLWidget::updateTextures(GLuint textureId,int width,int height,const uchar* data) {
	//绑定当前要操作的ID
	glBindTexture(GL_TEXTURE_2D,textureId);

	//上传纹理数据
	glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width,height,0,GL_RED,GL_UNSIGNED_BYTE,data);
}

void VideoOpenGLWidget::initializeGL() {
	qDebug() << "========initializeGL() Start=========";
	initializeOpenGLFunctions();

	//1.编译Shader
	m_program = new QOpenGLShaderProgram(this);
	m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, VERTEX_SHADER_SOURCE);
	m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, FRAGMENT_SHADER_SOURCE);
	if (!m_program->link()) {
		qDebug() << "Shader link failed:" << m_program->log();
	}

	//2.定义顶点坐标和纹理坐标
	static const GLfloat vertices[] = {//顶点坐标+纹理坐标
		-1.0f,-1.0f,0.0f,1.0f,//左下
		1.0f,-1.0f,1.0f,1.0f,//右下
		-1.0f,1.0f,0.0f,0.0f,//左上
		1.0f,1.0f,1.0f,0.0f//右上
	};
	m_vbo = new QOpenGLBuffer();
	m_vbo->create();//创建一个显存空间VBO
	m_vbo->bind();//绑定VBO到OpenGL
	m_vbo->allocate(vertices, sizeof(vertices));//将CPU数据拷贝到GPU显存中

	//3.初始化纹理ID
	initTextures();

	//设置背景为黑色
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	qDebug() << "========initializeGL() Done=========";
}

void VideoOpenGLWidget::resizeGL(int w,int h) {

	glViewport(0, 0, w, h);
}
void VideoOpenGLWidget::paintGL() {
	qDebug() << "paintGL() called!";
	glClear(GL_COLOR_BUFFER_BIT);
	m_mutex.lock();

	if (m_videoH == 0 || m_videoW == 0 || m_yData.isEmpty()) {
		m_mutex.unlock();
		return;
	}
	//激活shader
	if (!m_program->bind()) {
		m_mutex.unlock();
		return;
	}
	//设置顶点坐标和纹理坐标
	m_vbo->bind();
	int vertexLoc = m_program->attributeLocation("vertexIn");//获取attribute位置
	int vertexTex = m_program->attributeLocation("textureIn");
	m_program->enableAttributeArray(vertexLoc);//启用这两个attribute
	m_program->enableAttributeArray(vertexTex);
	m_program->setAttributeBuffer(vertexLoc, GL_FLOAT, 0, 2, sizeof(GLfloat) * 4);//描述数据布局（从VBO读取）
    m_program->setAttributeBuffer(vertexTex, GL_FLOAT, sizeof(GLfloat) * 2, 2, sizeof(GLfloat) * 4);


	//绑定纹理并上传数据(Y Texture+U Texture+V Texture)
	glActiveTexture(GL_TEXTURE0);
	updateTextures(m_textureIds[0],m_videoW,m_videoH,(const uchar*)m_yData.data());
	m_program->setUniformValue("tex_y",0);//将着色器绑定给ID

	glActiveTexture(GL_TEXTURE1);
	updateTextures(m_textureIds[1],m_videoW/2,m_videoH/2,(const uchar*)m_uData.data());
	m_program->setUniformValue("tex_u", 1);

	glActiveTexture(GL_TEXTURE2);
	updateTextures(m_textureIds[2], m_videoW / 2, m_videoH / 2, (const uchar*)m_vData.data());
	m_program->setUniformValue("tex_v",2);

	m_mutex.unlock();
	//顶点数据绘制为图元
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	m_program->disableAttributeArray(vertexLoc);
	m_program->disableAttributeArray(vertexTex);
	m_program->release();
	m_vbo->release();
}

void VideoOpenGLWidget::onFrameDecoded(const uchar* y, const uchar* u, const uchar* v,
									  int yStride, int uStride, int vStride,
									  int width, int height) {
	QMutexLocker locker(&m_mutex);

	//检查尺寸变化，重新分配内存
	if (m_videoW != width || m_videoH != height) {
		m_videoW = width;
		m_videoH = height;
		m_yData.resize(width*height);
		m_uData.resize(width*height/4);
		m_vData.resize(width*height/4);
	}

    //处理Stride，FFmpeg解码的数据含有填充字节,yStride>width;OpenGL希望数据紧凑，为避免花屏，需要逐步拷贝去除padding
	if (yStride == width) {
		memcpy(m_yData.data(),y,width*height);
	}
	else {
		//逐行拷贝
		uchar* dst = (uchar*)m_yData.data();
		for(int i=0;i<height;i++)
			memcpy(dst+i*width,y+i*yStride,width);    
	}
	
	int uvwidth = width / 2;
	int uvheight = height / 2;
	if (uStride == uvwidth) {
		memcpy(m_uData.data(),u,uvwidth*uvheight);
	}
	else {
		uchar* dst = (uchar*)m_uData.data();
		for (int i = 0; i < uvheight; i++) {
			memcpy(dst+i*uvwidth,u+i*uStride,uvwidth);
		}
	}

	if (vStride == uvwidth) {
		memcpy(m_vData.data(),v,uvwidth*uvheight);
	}
	else {
		uchar* dst = (uchar*)m_vData.data();
		for (int i = 0; i < uvheight; i++)
			memcpy(dst + i * uvwidth, v + i * vStride, uvwidth);
	}
	//触发UI重绘(调用paintGL)
	update();
}

void VideoOpenGLWidget::renderTestPattern(int patternType) {
	int width = 1280;
	int height = 720;
	QImage testImage;
	QString patternName;

	//根据类型生成不同的测试图片
	switch (patternType) {
		case 0:
			testImage = generateColorBars(width,height);
			patternName = "Color Bar";
			break;
		case 1:
			testImage = generateGradient(width,height);
			patternName = "Gradient";
			break;
		case 2:
			testImage = generateCheckerboard(width,height);
			patternName = "Checkerboard";
			break;
		case 3:
			testImage = generateTextPattern(width, height);
			patternName = "TextPattern";
			break;
		default:
			testImage = generateColorBars(width,height);
			patternName = "Default Bar";
			break;
	}	

	if (testImage.isNull()) {
		qWarning() << "Can not test picture";
		return;
	}
	qDebug() << "Generate picture" << patternName << "(" << width << "x" << height << ")";

	//显示图像
	displayImage(testImage);

}

void VideoOpenGLWidget::displayImage(const QImage& image) {
	if (image.isNull()) {
		qWarning() << "Picture is NULL";
		return;
	}

	//检查 OpenGL 是否已初始化
	if (!isValid()) {
		qWarning() << "OpenGL context is not valid yet!";
		return;
	}

	QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);

	qDebug() << "Start to convert to YUV480P";
	qDebug() << "Size_of_Picture：" << rgbImage.width() << "x" << rgbImage.height();
	qDebug() << "Raw Format：" << rgbImage.format();

	convertRGBtoYUV420P(rgbImage);

	update(); // 触发重绘

	qDebug() << "update() called, waiting for paintGL()...";
}

void VideoOpenGLWidget::convertRGBtoYUV420P(const QImage& image) {
	if (image.format() != QImage::Format_RGB888) {
		qWarning() << "Format of Picture must be RGB888,Current Format is：" << image.format();
		return;
	}
	QMutexLocker locker(&m_mutex);

	int width = image.width();
	int height = image.height();

	m_videoH = height;
	m_videoW = width;
	//分配YUV缓冲区
	int yStride = width * height;
	int uvStride = width * height / 4;

	m_yData.resize(yStride);
	m_uData.resize(uvStride);
	m_vData.resize(uvStride);

	uchar* yPlane = (uchar*)m_yData.data();
	uchar* uPlane = (uchar*)m_uData.data();
	uchar* vPlane = (uchar*)m_vData.data();
	//RGB转换为YUV，使用公式BT.601公式
	for (int y = 0; y < height; y++) {
		const uchar* rgbLine = image.scanLine(y);
		
		for (int x = 0; x < width; x++) {
			int r = rgbLine[x * 3 + 0];
			int g = rgbLine[x * 3 + 1];
			int b = rgbLine[x * 3 + 2];

			//计算Y分量
			int yValue = ((66*r+129*g+25*b+128)>>8) + 16;
			yPlane[y * width + x] = qBound(0, yValue, 255);
			//每2x2像素块计算一次U、V分量
			if (y % 2 == 0 && x % 2 == 0) {
				int r_sum = r, g_sum = g, b_sum = b;
				int count = 1;
				if (x + 1 < width) {
					r_sum += rgbLine[(x + 1) * 3+0];
					g_sum += rgbLine[(x + 1) * 3 + 1];
					b_sum += rgbLine[(x + 1) * 3 + 2];
					count++;
				}
				
				if (y + 1 < height) {
					const uchar* nextLine = image.scanLine(y + 1);
					r_sum += nextLine[x * 3 + 0];
					g_sum += nextLine[x * 3 + 1];
                    b_sum += nextLine[x * 3 + 2];
					count++;
					if (x + 1 < width) {
						r_sum+=nextLine[(x + 1) * 3 + 0];
						g_sum+=nextLine[(x + 1) * 3 + 1];
                        b_sum+=nextLine[(x + 1) * 3 + 2];
						count++;
					}
				}
				r = r_sum / count;
				g = g_sum / count;
				b = b_sum / count;

				int uValue=((-38*r-74*g+112*b+128)>>8) + 128;
				int vValue=((112*r-94*g-18*b+128)>>8) + 128;

				int uvIndex = (y / 2) * (width / 2) + (x / 2);
				uPlane[uvIndex] = qBound(0,uValue,255);
				vPlane[uvIndex] = qBound(0,vValue,255);
			}

		}
	}

	qDebug() << "RGBConvert to YUV420P Succeed";
	qDebug()<<"Size of video："<<m_videoW<<"x"<<m_videoH;
	qDebug() << "Y Size" << m_yData.size() << "byte";
	qDebug() << "U Size" << m_uData.size() << "byte";
    qDebug() << "V Size" << m_vData.size() << "byte";


}

QImage VideoOpenGLWidget::generateColorBars(int width,int height) {
	QImage image(width,height,QImage::Format_RGB888);
	//标准七色彩条
	QColor colors[7] = {
		QColor(255,255,255),
		QColor(255,255,0),
		QColor(0,255,255),
		QColor(0,255,0),
		QColor(255,0,255),
		QColor(255,0,0),
		QColor(0,0,255)
	};
	int barWidth = width / 7;
	for (int y = 0; y < height; y++) {
		uchar* line = image.scanLine(y);
		for (int x = 0; x < width; x++) {
			int colorIndex = qMin(x/barWidth,6);
			QColor color = colors[colorIndex];
			
			line[x * 3 + 0] = color.red();
			line[x * 3 + 1] = color.green();
			line[x * 3 + 2] = color.blue();
		}
	}
	QPainter painter(&image);
	painter.setPen(Qt::black);
	painter.setFont(QFont("Arial",48,QFont::Bold));
	painter.drawText(QRect(0,50,width,100),Qt::AlignCenter,"VideoOpenGLWidgetColor Bar Test");
	//添加时间戳
	painter.setPen(Qt::darkGray);
	painter.setFont(QFont("Arial",24));
	QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	painter.drawText(QRect(0,height-80,width,50),Qt::AlignCenter,timestamp);

	return image;
}

QImage VideoOpenGLWidget::generateGradient(int width,int height) {
	QImage image(width,height,QImage::Format_RGB888);
	for (int y = 0; y < height; y++) {
		uchar* line = image.scanLine(y);
		for (int x = 0; x < width; x++) {
			int r = (x * 255) / width;
			int g = ((x + y) * 255) / (width + height);
			int b = (y * 255) / height;

			line[x * 3 + 0] = qBound(0,r,255);
			line[x * 3 + 1] = qBound(0,g,255);
			line[x * 3 + 2] = qBound(0, b, 255);
		}
	}

	QPainter painter(&image);
	painter.setPen(Qt::black);
	painter.setFont(QFont("Arial",48,QFont::Bold));
	painter.drawText(QRect(0,height/2-30,width,60),Qt::AlignCenter,"RGB Gradient Test");

	return image;
}

QImage VideoOpenGLWidget::generateCheckerboard(int width,int height) {
	QImage image(width,height,QImage::Format_RGB888);
	int block_size = 64;
	for (int y = 0; y < height; y++) {
		uchar* line = image.scanLine(y);
		for (int x = 0; x < width; x++) {
			int blockX = x / block_size;
			int blockY = y / block_size;
			bool isWhite = (blockX + blockY) % 2 == 0;

			uchar value = isWhite ? 255 : 0;
			line[x*3+0]=value;
			line[x*3+1]=value;
			line[x*3+2]=value;
		}
	}
	QPainter painter(&image);
	painter.setPen(Qt::red);
	painter.setFont(QFont("Arial", 48, QFont::Bold));
	painter.drawText(QRect(0, 50, width, 100), Qt::AlignCenter, "Checker Board Test");

	return image;
}


QImage VideoOpenGLWidget::generateTextPattern(int width, int height) {
	QImage image(width,height,QImage::Format_RGB888);
	image.fill(QColor(40,40,60));
	QPainter painter(&image);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);

	painter.setPen(QPen(Qt::green, 3));
	painter.drawRect(5, 5, width - 10, height - 10);
	//绘制各种大小的文字
	QStringList texts = {
		"VideoOpenGLWidget Test",
		"YUV420P->RGB Test",
		"OpenGL Shader Test",
		"Qt+OpenGL Test",
		QString("Resolution:%1x%2").arg(width).arg(height)
	};


	QList<int> fontSize = {56,42,32,28,24};
	QList<QColor> colors = {Qt::white,Qt::yellow,Qt::cyan,QColor(100,255,100),Qt::red};

	for (int i = 0; i < texts.size(); i++) {
		painter.setPen(colors[i]);
		painter.setFont(QFont("Arial",fontSize[i],QFont::Bold));

		int y = 120 + i * 120;
		painter.fillRect(QRect(100, y, width - 200, 80), QColor(0, 0, 0, 50));
		painter.drawText(QRect(100,y,width-200,80),Qt::AlignLeft|Qt::AlignVCenter,texts[i]);
	}

	painter.setPen(Qt::lightGray);
	painter.setFont(QFont("Arial",20));
	QString timestamp=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
	painter.drawText(QRect(0,height-60,width,40),Qt::AlignCenter,QString("Generate Time:%1").arg(timestamp));
	return image;
}
