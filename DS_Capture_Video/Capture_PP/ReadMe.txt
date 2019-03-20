
DriectShow：
#include<dshow.h>
#pragma comment(lib,"Strmiids.lib")


流程介绍：

CCapture_PPDlg：为主界面类.
CaptureManager：为操作Video和Audio的中间类
VideoCapture：为Video功能类
Effect：为特效类
RtpPacket：为打包成Rtp格式的类
SampleCallBack：为DirectShow进行回调的Sample类.

DriectShow 视频采集流程：
1）枚举设备 
ICreateDevEnum	*m_pDevEnum;
CoCreateInstance(CLSID_SystemDeviceEnum, 
				NULL, 
				CLSCTX_INPROC_SERVER, 
				IID_ICreateDevEnum, 
				(void **)&m_pDevEnum);

2)通过枚举设备获取视频采集设备枚举
IEnumMoniker	*m_pEnumMoniker;
CreateClassEnumerator(
					CLSID_VideoInputDeviceCategory, 
					&m_pEnumMoniker, 
					0)
3）创建Graph管理对象（FilterGraph）
IGraphBuilder	*m_pGraphBuilder;
CoCreateInstance(
					CLSID_FilterGraph, 
					NULL, 
					CLSCTX_INPROC_SERVER, 
					IID_IFilterGraph, 
					(void **)&m_pGraphBuilder);
4）创建一个采集Graph
ICaptureGraphBuilder2	*m_pCaptureGraph;
CoCreateInstance(
				CLSID_CaptureGraphBuilder2,
				NULL, 
				CLSCTX_INPROC_SERVER, 
				IID_ICaptureGraphBuilder2, 
				(void **)&m_pCaptureGraph);
5）把采集Graph设置加入到Graph管理对象中
m_pCaptureGraph->SetFiltergraph(m_pGraphBuilder);

6）通过管理对象获取Graph控制对象
IMediaControl	*m_pMediaControl;
m_pGraphBuilder->QueryInterface(
				IID_IMediaControl,
				(void **)&m_pMediaControl);

7）通过管理对象获取视频输出预览窗体
IVideoWindow	*m_pVideoWindow;
m_pGraphBuilder->QueryInterface(
				IID_IVideoWindow,
				(void **)&m_pVideoWindow);

主要流程为graph管理器管理采集的Graph，而采集的Graph里为采集，传输，预览等filter.				
以上工作为graph管理器的创建，以及采集Graph的创建，接下来需要创建在Graph里连接的filter.

接下来就是要创建filter,并且连接filter.主要分为三个fitler，
采集filter，Grabberfilter，Renderfilter,主要的数据处理需要在Grabberfilter内部实现.

8)通过选择的采集设备创建一个采集filter，并加载入采集Graph
通过枚举的采集设备查找主要方法如下：
m_pEnumMoniker->Next(1, &pMoniker, &ulFected);
pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void **)&pProBag)
hr = pProBag->Read(L"FriendlyName", &varName, 0);
pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void **)&pFilter);

m_pGraphBuilder->AddFilter(m_pCaptureFilter, L"Capture Filter");

9）获取Grabberfilter，并加载入采集Graph
IBaseFilter		*m_pGrabberFilter;
CoCreateInstance(
				CLSID_SampleGrabber, 
				NULL, 
				CLSCTX_INPROC_SERVER, 
				IID_IBaseFilter, 
				(void **)&m_pGrabberFilter);
m_pGraphBuilder->AddFilter(m_pGrabberFilter, L"Grabber Filter");
此时需要创建一个Grabber实例用于实现数据的处理.
ISampleGrabber	*m_pSampleGrabber;
m_pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void **)&m_pSampleGrabber);
通过设置回调函数用于数据的回调ISampleGrabberCB

10）获取Renderfilter，并加载入采集Graph
CoCreateInstance(
			CLSID_NullRenderer, 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			IID_IBaseFilter, 
			(void **)&m_pNullRenderFilter);
m_pGraphBuilder->AddFilter(m_pNullRenderFilter, L"NullRenderer")

11）以上为filter创建完成并且都加载劲采集Graph内部，这一步需要连接filter
m_pCaptureGraph->RenderStream(
						&PIN_CATEGORY_PREVIEW,
						&MEDIATYPE_Video, 
						m_pCaptureFilter, 
						NULL, 
						NULL);	
m_pCaptureGraph->RenderStream(&PIN_CATEGORY_CAPTURE,
						&MEDIATYPE_Video, 
						m_pCaptureFilter, 
						m_pGrabberFilter, 
						m_pNullRenderFilter);

12）预览窗体和本地窗体句柄进行一个绑定
m_pVideoWindow->put_Owner((OAHWND)m_hwnd)
m_pVideoWindow->put_WindowStyle(WS_CHILD)
m_pVideoWindow->SetWindowPosition(0, 0, re.right - re.left, re.bottom - re.top)
m_pVideoWindow->put_Visible(OATRUE)

13）采集开始
m_pMediaControl->Run();

14）采集信息设置
IAMStreamConfig *m_pConfig;
m_pCaptureGraph->FindInterface(&PIN_CATEGORY_CAPTURE,
						&MEDIATYPE_Video,
						m_pCaptureFilter,
						IID_IAMStreamConfig,
						(void **)&m_pConfig）
												
m_pConfig->SetFormat(amt)

						
以上的重点为GrabberFilter实现对象的创建.利用了网上开源的qedit.h 
########################################################################################################












