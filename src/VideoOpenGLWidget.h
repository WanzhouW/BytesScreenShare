#pragma once
#ifndef VIDEOOPENGL_H
#define VIDEOOPENGL_H
#include<QOpenGLWidget>
#include<QOpenGLFunctions>
#include<QOpenGLShaderProgram>
#include<QOpenGLBuffer>
#include<QOpenGLTexture>
#include<QMutex>
#include<QImage>


class VideoOpenGLWidget :public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT
public:
	explicit VideoOpenGLWidget(QWidget* parent = nullptr);
	~VideoOpenGLWidget();

public slots:
	//接受解码后的YUV数据，槽函数在子进程中调用，需要内部加锁并拷贝数据
	void onFrameDecoded(const uchar* y, const uchar* u, const uchar* v,
						int yStride,int uStride,int vStride,
						int width,int height);
public:
	//新增：用于测试渲染的方法
	void renderTestPattern(int patternType=0);
	void displayImage(const QImage& image);



protected:
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int w, int h) override;
private:
	//OpenGL资源
	QOpenGLShaderProgram* m_program = nullptr;//着色器程序
	QOpenGLBuffer *m_vbo = nullptr;//顶点缓存对象

	//YUV的三个纹理ID
	GLuint m_textureIds[3] = {0,0,0};

	//数据缓存，将数据从FFmpeg中拷贝到这，防止渲染前被覆盖
	QByteArray m_yData;
	QByteArray m_uData;
	QByteArray m_vData;

	int m_videoW = 0;
	int m_videoH = 0;

	//互斥锁：保护上面的Data以及H/W值
	QMutex m_mutex;

	//标记是否被初始化纹理
	bool m_texturesCreated = false;

	//辅助函数：初始化纹理配置
	void initTextures();
	//辅助函数：更新纹理数据
	void updateTextures(GLuint textureId,int width,int height,const uchar* data);

	//新增：生成测试图片的函数
	QImage generateColorBars(int width, int heigth);
	QImage generateGradient(int width, int height);
	QImage generateCheckerboard(int width, int height);
	QImage generateTextPattern(int width,int height);

	//新增：RGB转为YUV420P
	void convertRGBtoYUV420P(const QImage& image);
};



#endif