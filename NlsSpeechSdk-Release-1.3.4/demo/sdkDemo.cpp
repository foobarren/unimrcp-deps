#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "pthread.h"
#include "NlsClient.h"	
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <algorithm>
#include <unistd.h>
using std::min;
#endif

#define FRAME_SIZE 6400

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::exception;

/*自定义线程参数*/
struct ParamStruct {
	string fileName;
	string config;
	string id;
	string scret;
};


/*自定义事件回调参数*/
struct ParamCallBack {
	int iExg;
	string sExg;
};

/**
* @brief 获取sendAudio发送延时时间
* @param dataSize 待发送数据大小
* @param sampleRate 采样率 16k/8K
* @param compressRate 数据压缩率，例如压缩比为10:1的16k opus编码，此时为10；非压缩数据则为1
* @return 返回sendAudio之后需要sleep的时间
* @note 对于8k pcm 编码数据, 16位采样，建议每发送3200字节 sleep 200 ms.
对于16k pcm 编码数据, 16位采样，建议每发送6400字节 sleep 200 ms.
对于其它编码格式的数据, 用户根据压缩比, 自行估算, 比如压缩比为10:1的 16k opus,
需要每发送6400/10=640 sleep 200ms.
*/
unsigned int getSendAudioSleepTime(const int dataSize,
	const int sampleRate,
	const int compressRate) {
	/*仅支持16位采样*/
	const int sampleBytes = 16;
	/*仅支持单通道*/
	const int soundChannel = 1;

	/*当前采样率，采样位数下每秒采样数据的大小*/
	int bytes = (sampleRate * sampleBytes * soundChannel) / 8;

	/*当前采样率，采样位数下每毫秒采样数据的大小*/
	int bytesMs = bytes / 1000;

	/*待发送数据大小除以每毫秒采样数据大小，以获取sleep时间*/
	int sleepMs = (dataSize * compressRate) / bytesMs;

	return sleepMs;
}

/*
*在回调函数内部,请不要进行阻,耗时过久操作.
*在接收到服务端识别结果响应时,上报OnResultDataRecved回调.此时可以在回调函数内部读取识别结果.
*注意：请不要在回调函数内部调用stop, releaseRequest操作。
*/
void OnResultDataRecved(NlsEvent* cbEvent, void* cbParam) {
	ParamCallBack* tmpParam = (ParamCallBack*)cbParam;

	cout << "CbParam: " << tmpParam->iExg << " " << tmpParam->sExg << endl; /*仅表示自定义参数示例*/

	cout << "OnResultDataRecved: " << cbEvent->getResponse() << endl; /*getResponse() 可以获取云端响应信息*/
}

/*
*在回调函数内部,请不要进行阻,耗时过久操。
*request内部流程出错,上报OnOperationFailed回调，此时可以停止工作线程send操作,调用stop。
*注意：请不要在回调函数内部调用stop, releaseRequest操作。
*/
void OnOperationFailed(NlsEvent* cbEvent, void* cbParam) {

	ParamCallBack* tmpParam = (ParamCallBack*)cbParam;

	cout << "CbParam: " << tmpParam->iExg << " " << tmpParam->sExg << endl; /*仅表示自定义参数示例*/

	cout << "OnOperationFailed: " << cbEvent->getErrorMessage() << endl; /*getErrorMessage() 可以获取异常失败信息*/
}

/*
*在回调函数内部,请不要进行阻,耗时过久操.
*在调用stop(),或者服务端主动关闭连接通道时,上报OnChannelCloseed回调,通知连接通道关闭.
*注意：请不要在回调函数内部调用stop, releaseRequest操作。
*/
void OnChannelCloseed(NlsEvent* cbEvent, void* cbParam) {
	ParamCallBack* tmpParam = (ParamCallBack*)cbParam;

	cout << "CbParam: " << tmpParam->iExg << " " << tmpParam->sExg << endl; /*仅表示自定义参数示例*/

	cout << "OnChannelCloseed: " << cbEvent->getResponse() << endl; /*getResponse() 可以通道关闭信息*/
}

void* pthreadFunc(void* arg) {
	int sleepMs = 0;
	ParamCallBack cbParam;
	NlsSpeechCallback* callback = NULL;

	/*0: 从自定义线程参数中获取id, scret等配置文件等参数.*/
	ParamStruct* tst = (ParamStruct*)arg;
	if (tst == NULL) {
		cout << "arg is not valid." << endl;
		return NULL;
	}

	/* 打开音频文件, 获取数据 */
	ifstream fs;
	fs.open(tst->fileName.c_str(), ios::binary | ios::in);
	if (!fs) {
		cout << tst->fileName << " isn't exist.." << endl;

		return NULL;
	}

	/*初始化自定义回调参数, 仅作为示例表示参数传递, 在demo中不起任何作用*/
	cbParam.iExg = 1;
	cbParam.sExg = "exg.";

	/*
	* 1: 创建并设置回调函数
	*/
	callback = new NlsSpeechCallback();
	callback->setOnMessageReceiced(OnResultDataRecved, &cbParam);
	callback->setOnOperationFailed(OnOperationFailed, &cbParam);
	callback->setOnChannelClosed(OnChannelCloseed, &cbParam);

	/***************以读取config.txt方式创建request****************/
	/*
	* 创建一句话识别  NlsRequest,第一个参数为callback对象,第二个参数为config.txt文件.
	* request对象在一个会话周期内可以重复使用.
	* 会话周期是一个逻辑概念. 比如Demo中,指读取, 发送完整个音频文件数据的时间.
	* 音频文件数据发送结束时, 可以releaseNlsRequest.
	* request方法调用,比如create, start, sendAudio, stop, release必须在.
	* 同一线程内完成,跨线程使用可能会引起异常错误.
	*/
	/*
	* 2: 创建一句话识别createRealTimeRequest对象
	*/
	NlsRequest* request = NlsClient::getInstance()->createRealTimeRequest(callback, tst->config.c_str());
	/***********************************************************/

	/*****************以参数设置方式创建request******************
	NlsRequest* request = gNlc->createRealTimeRequest(gCallback, NULL);
	request->SetParam("AppKey","your-key");
	request->SetParam("SampleRate","8000");
	request->SetParam("ResponseMode","streaming");
	request->SetParam("Format","pcm");
	request->SetParam("Url","wss://nls-trans.dataapi.aliyun.com:443/realtime");
	************************************************************/
	if (request == NULL) {
		cout << "createRealTimeRequest fail" << endl;

		delete callback;
		callback = NULL;

		return NULL;
	}

	request->Authorize(tst->id.c_str(), tst->scret.c_str());

	/*
	* 3: start()为阻塞操作, 发送start指令之后, 会等待服务端响应, 或超时之后才返回
	*/
	if (request->Start() < 0) {
		cout << "start() failed." << endl;

		NlsClient::getInstance()->releaseNlsRequest(request); /*start()失败，释放request对象*/

		delete callback;
		callback = NULL;

		return NULL;
	}

	while (!fs.eof()) {
		char data[FRAME_SIZE] = { 0 };

		fs.read(data, sizeof(char) * FRAME_SIZE);
		int nlen = fs.gcount();

		/*
		* 4: 发送音频数据. sendAudio返回-1表示发送失败, 需要停止发送. 对于第三个参数:
		* request对象format参数为pcm时, 使用false即可. format为opu, 使用压缩数据时, 需设置为true.
		*/
		nlen = request->SendAudio(data, nlen);
		if (nlen < 0) {
			/*发送失败, 退出循环数据发送*/
			cout << "send data fail." << endl;
			break;
		}
		else {
			cout << "send len:" << nlen << " ." << endl;
		}

		/*
		*语音数据发送控制：
		*语音数据是实时的, 不用sleep控制速率, 直接发送即可.
		*语音数据来自文件, 发送时需要控制速率, 使单位时间内发送的数据大小接近单位时间原始语音数据存储的大小.
		*/
		sleepMs = getSendAudioSleepTime(6400, 16000, 1); /*根据 发送数据大小，采样率，数据压缩比 来获取sleep时间*/

														 /*
														 * 5: 语音数据发送延时控制
														 */
#if defined(_WIN32)
		Sleep(sleepMs);
#else
		usleep(sleepMs * 1000);
#endif
	}

	/* 关闭音频文件 */
	fs.close();

	/*
	*6: 数据发送结束，关闭识别连接通道.
	*stop()为阻塞操作, 在接受到服务端响应, 或者超时之后, 才会返回.
	*/
	request->Stop();

	/*7: 识别结束, 释放request对象*/
	NlsClient::getInstance()->releaseNlsRequest(request);

	return NULL;
}

/**
* 线程循环识别
* 需要调整count值和每次要识别的文件，Demo中默认每次识别一个文件
*/
void* multiRecognize(void* arg) {
	int count = 2;
	while (count > 0) {
		pthreadFunc(arg);
		count--;
	}

	return NULL;
}

/**
* 识别单个音频数据
*/
int speechRealTimeFile(const char* configFile, const char* id, const char* scret) {

	ParamStruct pa;
	pa.config = configFile;
	pa.id = id;
	pa.scret = scret;
	pa.fileName = "test0.wav";

	pthread_t pthreadId;

	/*启动一个工作线程, 用于单次识别*/
	pthread_create(&pthreadId, NULL, &pthreadFunc, (void *)&pa);

	/*启动一个工作线程, 用于循环识别*/
	// pthread_create(&pthreadId, NULL, &multiRecognize, (void *)&pa);

	pthread_join(pthreadId, NULL);

	return 0;

}

/**
* 识别多个音频数据;
* sdk多线程指一个音频数据源对应一个线程, 非一个音频数据对应多个线程.
* 示例代码为同时开启4个线程识别4个文件;
* 免费用户并发连接不能超过10个;
*/
#define AUDIO_FILE_NUMS 4
#define AUDIO_FILE_NAME_LENGTH 32
int speechRealTimeMultFile(const char* configFile, const char* id, const char* scret) {

	char audioFileNames[AUDIO_FILE_NUMS][AUDIO_FILE_NAME_LENGTH] = { "test0.wav", "test1.wav", "test2.wav", "test3.wav" };
	ParamStruct pa[AUDIO_FILE_NUMS];

	for (int i = 0; i < AUDIO_FILE_NUMS; i++) {
		pa[i].config = configFile;
		pa[i].id = id;
		pa[i].scret = scret;
		pa[i].fileName = audioFileNames[i];
	}

	vector<pthread_t> pthreadId(AUDIO_FILE_NUMS);
	/*启动四个工作线程, 同时识别四个音频文件*/
	for (int j = 0; j < AUDIO_FILE_NUMS; j++) {
		pthread_create(&pthreadId[j], NULL, &pthreadFunc, (void *)&(pa[j]));
	}

	for (int j = 0; j < AUDIO_FILE_NUMS; j++) {
		pthread_join(pthreadId[j], NULL);
	}

	return 0;
}

int main(int arc, char* argv[]) {
	if (arc < 4) {
		cout << "params is not valid. Usage: ./demo config.txt your-id your-scret" << endl;
		return -1;
	}

	int ret = NlsClient::getInstance()->setLogConfig("log-realtime.txt", LogInfo);
	if (-1 == ret) {
		cout << "set log failed." << endl;
		return -1;
	}

	/*识别单个音频数据*/
	speechRealTimeFile(argv[1], argv[2], argv[3]);

	/*识别多个音频数据*/
	//speechRealTimeMultFile(argv[1], argv[2], argv[3]);

	/*所有工作完成，进程退出前，释放nlsClient. 请注意, releaseInstance()非线程安全.*/
	NlsClient::releaseInstance();

	return 0;
}
