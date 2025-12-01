# WebRTC 信令服务器信令文档

本文档定义了客户端（Peer）与信令服务器（Signaling Server）之间通过 WebSocket 协议进行通信的 JSON 消息格式。

服务器采用**单房间、直接路由**模式。

## 1. 通用消息结构

所有信令消息均采用统一的 JSON 格式封装。

| **字段** | **类型** | **描述**                                                  | **必需性**  |
| ------ | ------ | ------------------------------------------------------- | -------- |
| `type` | String | 消息的类型。                                                  | 必需       |
| `from` | String | 消息发送者的唯一 ID。**注意：在 `REGISTER_REQUEST` 消息中，此字段可以省略或置空。** | **条件必需** |
| `to`   | String | 消息接收者的唯一 ID。若发送给服务器，值为 `"Server"`。                      | 必需       |
| `data` | Object | 消息的具体数据载荷。                                              |          |

## 2. 信令消息类型详情 (`SignalingType`)

### 2.1. 客户端到服务器 (C → S)
#### 2.1.1. `REGISTER_REQUEST` (请求注册)

客户端首次连接后发送此消息，请求服务器为其分配一个唯一的 Peer ID。

| **字段** | **描述**                 |
| ------ | ---------------------- |
| `type` | `"REGISTER_REQUEST"`   |
| `from` | **此字段可省略**。客户端此时尚无 ID。 |
| `to`   | `"Server"`             |
| `data` | **此字段可省略**。            |


**示例 (C → S):**
```JSON
{
  "type": "REGISTER_REQUEST",
  "to": "Server",
  "data": {}
}
```

#### 2.1.2. `OFFER` (发送会话提议)

WebRTC 连接建立的第一步，客户端 A 向 B 发送 SDP Offer。

| 字段 | 描述 |
| :--- | :--- |
| `type` | `"OFFER"` |
| `from` | 发送方 ID (发起连接的 Peer) |
| `to` | **目标 Peer 的 ID** |
| `data` | 包含 `sdp` 字段，内容为 SDP Offer 字符串。 |

**示例 (C → S → C):**

```json
{
  "type": "OFFER",
  "from": "Peer_A",
  "to": "Peer_B",
  "data": {
    "sdp": "v=0\r\no=- 5236683884878235471 2 IN IP4 127.0.0.1\r\n..."
  }
}
```

#### 2.1.3. `ANSWER` (发送会话应答)

客户端 B 收到 Offer 后，处理并向 A 发送 SDP Answer。

| 字段 | 描述 |
| :--- | :--- |
| `type` | `"ANSWER"` |
| `from` | 发送方 ID (接收 Offer 的 Peer) |
| `to` | **目标 Peer 的 ID** (即 Offer 的发送方) |
| `data` | 包含 `sdp` 字段，内容为 SDP Answer 字符串。 |

**示例 (C → S → C):**

```json
{
  "type": "ANSWER",
  "from": "Peer_B",
  "to": "Peer_A",
  "data": {
    "sdp": "v=0\r\no=- 5236683884878235471 2 IN IP4 127.0.0.1\r\n..."
  }
}
```

#### 2.1.4. `ICE` (发送 ICE 候选者)

用于在 NAT 穿透过程中交换网络地址信息。

| 字段 | 描述 |
| :--- | :--- |
| `type` | `"ICE"` |
| `from` | 发送方 ID |
| `to` | **目标 Peer 的 ID** |
| `data` | 包含 `candidate` (完整的 ICE 字符串) 和 `sdpMid`, `sdpMLineIndex` 等字段。 |

**示例 (C → S → C):**

```json
{
  "type": "ICE",
  "from": "Peer_A",
  "to": "Peer_B",
  "data": {
    "candidate": "candidate:123 1 udp 2122266859 192.168.1.10 50000 typ host",
    "sdpMid": "audio",
    "sdpMLineIndex": 0
  }
}
```



### 2.2. 服务器到客户端 (S → C)

#### 2.2.1. `REGISTER_SUCCESS` (注册成功/获取列表)

服务器对 `REGISTER_REQUEST` 的肯定回复。**此消息包含服务器为客户端分配的正式 Peer ID。**

|**字段**|**描述**|
|---|---|
|`type`|`"REGISTER_SUCCESS"`|
|`from`|`"Server"`|
|`to`|注册成功的客户端的 **`ClientSession` 内部 ID** (仅本次传输用，由服务器内部确定接收方)。|
|`data`|包含 `peerId`（服务器分配的 ID）和当前房间内所有其他 Peer 的列表。|

**示例 (S → C):**

```JSON
{
  "type": "REGISTER_SUCCESS",
  "from": "Server",
  "to": "UUID-12345", 
  "data": {
    "peerId": "UUID-12345", // <-- 服务器分配给客户端的正式 ID
    "message": "Welcome to the room!",
    "peers": ["UUID-12345", "UUID-23456", "UUID-6666"] // 当前房间内所有其他 Peer
  }
}
```


#### 2.2.2. `PEER_JOINED` (新 Peer 加入通知)

当有新的客户端成功注册后，服务器向所有现有客户端广播此消息。

| 字段 | 描述 |
| :--- | :--- |
| `type` | `"PEER_JOINED"` |
| `from` | `"Server"` |
| `to` | **广播（所有 Peer）** |
| `data` | 包含新加入 Peer 的 ID 和元数据。 |

**示例 (S → C，广播):**

```json
{
  "type": "PEER_JOINED",
  "from": "Server",
  "to": "All",
  "data": {
    "id": "UUID-12345"
  }
}
```

#### 2.2.3. `PEER_LEFT` (Peer 离开通知)

当客户端主动断开 WebSocket 或服务器检测到连接丢失时，服务器向所有现有客户端广播此消息。

| 字段 | 描述 |
| :--- | :--- |
| `type` | `"PEER_LEFT"` |
| `from` | `"Server"` |
| `to` | **广播（所有 Peer）** |
| `data` | 包含离开 Peer 的 ID。 |

**示例 (S → C，广播):**

```json
{
  "type": "PEER_LEFT",
  "from": "Server",
  "to": "All",
  "data": {
    "id": "Peer_E"
  }
}
```



### 2.3. 错误/通用消息

#### 2.3.1. `ERROR_MESSAGE` (错误通知)

服务器向客户端发送的错误或警告信息。

| 字段 | 描述 |
| :--- | :--- |
| `type` | `"ERROR_MESSAGE"` |
| `from` | `"Server"` |
| `to` | 发生错误的客户端 ID |
| `data` | 包含 `code` 和 `message`。 |

**示例 (S → C):**

```json
{
  "type": "ERROR_MESSAGE",
  "from": "Server",
  "to": "Peer_A",
  "data": {
    "message": "Target Peer_Z not found in the room."
  }
}
```