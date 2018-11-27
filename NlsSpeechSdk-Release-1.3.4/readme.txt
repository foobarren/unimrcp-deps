语音智能服务C++ SDK1.0

接入前请仔细阅读C++ SDK1.0文档：https://help.aliyun.com/document_detail/30437.html


SDK压缩包说明:

- CMakeLists.txt demo工程的CMakeList文件
- build 编译目录
- demo 包含demo.cpp，各语音服务配置文件。各文件描述见下表：

| 文件名  | 描述  |
| ------------ | ------------ |
| sdkDemo.cpp |
windows专用，默认为实时长语音识别功能demo，如需可自行替换成其它功能(编码格式：UTF-8
代签名) |
| config-asr.txt |  一句话识别配置文件 |
| config-tts.txt | 语音合成配置文件 |
| config-realtime.txt | 实时音频流识别配置文件  |
|  asr-demo.cpp | 一句话识别demo  |
|  tts-demo.cpp | 语音合成demo  |
|  realtime-demo.cpp | 实时音频流识别demo  |
|  testX.wav | 测试音频(格式：16K采样率，单声道，PCM)  |

- include 包含sdk头文件，以及部分第三方头文件。各文件描述见下表

| 文件名  | 描述  |
| ------------ | ------------ |
| openssl |  openssl  |
| pthread | pthread线程（windows下使用） |
| opus |  opus  |
| nlsClient.h | SDK  |

- lib
  包含sdk，以及第三方依赖库。linux.tar.gz为linux版本动态库压缩包。windows.zip为windows版本动态库压缩包。其中根据平台不同，
可以选择linux版本32/64位librealTimeUnity.so(glibc2.5及以上, Gcc4, Gcc5), windows版本x86/64位realTimeSdk.dll（VS2013、VS2015）
- readme.txt SDK说明
- release.log 版本说明
- version 版本号
- build.sh demo编译脚本

注意：
1. linux环境下，运行环境最低要求：Glibc 2.5及以上， Gcc4、Gcc5
2. windows下，目前支持VS2013，VS2015


依赖库：
SDK 依赖
openssl(l-1.0.2j)，opus(1.2.1)，pthread(2.9.1, windows下使用。linux下系统自带pthread)。依赖库放置在
path/to/sdk/lib 下。
注意：path/to/sdk/lib/windwos/1x.0/pthread仅在windows下使用。
      SDK压缩包内提供的依赖库为64位，不提供32位。在32位下，需要用户自行编译。


Linux下demo编译过程:

准备工作：
1: cd path/to/sdk/lib
2: tar -zxvpf linux.tar.gz

cmake编译：
1: 请确认本地系统以安装Cmake，最低版本2.6
2: 执行[cd path/to/sdk/ && ./build.sh]编译demo
3: 编译完毕，进入path/to/sdk/demo目录，执行[./demo config-XXX.txt your-id your-scret]

如果不支持cmake，可尝试手动编译:
1: g++ -o demo/asrDemo demo/asr-demo.cpp -I./include -L./lib/linux -lrealTimeUnity -lssl -lcrypto -lopus -lpthread -D_GLIBCXX_USE_CXX11_ABI=0
2: export LD_LIBRARY_PATH=./lib/linux/
3: ./demo/asrDemo config-xxx.txt your-id your-scret


Windows下demo编译工程：

Windows下需要用户自己搭建VS工程。

准备工作：
1: 进入 path/to/sdk/lib
2: 解压windows.zip


