#include "OnvifNetwork.h"
#include "guid.h"
#include <QtXml>
#include <QTextStream>

#include <QDebug>

#define qDebug() qDebug()<<__FUNCTION__<<__LINE__

#define FORMATE_FOUR_PARAM(str, arr)\
	str = str.sprintf("%d.%d.%d.%d", arr[0], arr[1], arr[2], arr[3]);

#define FORMATE_SIX_PARAM(str, arr)\
	str = str.sprintf("%x:%x:%x:%x:%x:%x", arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);



OnvifNetwork::OnvifNetwork():
	m_nRef(0),
	m_nConfigNum(0)
{
	m_pNvpContext = NVP_ONVIF_new();
}

OnvifNetwork::~OnvifNetwork()
{
	if (m_pNvpContext){
		NVP_ONVIF_delete(m_pNvpContext);
		m_pNvpContext = NULL;
	}
}

long __stdcall OnvifNetwork::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IPcomBase == iid)
	{
		*ppv=static_cast<IPcomBase *>(this);
	}
	else if (IID_IAutoSycTime == iid)
	{
		*ppv=static_cast<IAutoSycTime *>(this);
	}
	else if (IID_IOnvifRemoteInfo == iid)
	{
		*ppv = static_cast<IOnvifRemoteInfo *>(this);
	}
	else{
		*ppv=NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();
	return S_OK;
}

unsigned long __stdcall OnvifNetwork::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall OnvifNetwork::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}

int OnvifNetwork::setAutoSycTime( bool bEnabled )
{
	if (bEnabled){
		if (!m_pNvpContext){
			qDebug()<<"get nvp interface fail";
			return 1;
		}
		int ret = m_pNvpContext->SetSystemDateTime(&m_nvpArguments, NULL);
		if (ret){
			return 1;
		}
	}
	return 0;
}

void OnvifNetwork::setOnvifDeviceInfo( const QString &sIp, const QString &sPort, const QString &sUserName, const QString &sPassword )
{
	NVP_INIT_ARGS2(m_nvpArguments, (void*)&m_nvpArguments, sIp.toLatin1().data(), sPort.toInt(), sUserName.toLatin1().data(), sPassword.toLatin1().data(), 0);
}

bool OnvifNetwork::getOnvifDeviceNetworkInfo( QString &sMac,QString &sGateway,QString &sMask,QString &sDns )
{
	if (!m_pNvpContext){
		qDebug()<<"nvp interface is NULL";
		return false;
	}
	stETHER_CONFIG ethCfg;
	qMemSet(&ethCfg, 0, sizeof(stETHER_CONFIG));
	int ret = m_pNvpContext->GetNetworkInterface(&m_nvpArguments, (lpNVP_ETHER_CONFIG)&ethCfg);
	if (ret){
		qDebug()<<"get network config fail";
		return false;
	}
	FORMATE_FOUR_PARAM(sMask, ethCfg.netmask);
	FORMATE_FOUR_PARAM(sGateway, ethCfg.gateway);
	FORMATE_FOUR_PARAM(sDns, ethCfg.dns1);
	FORMATE_SIX_PARAM(sMac, ethCfg.mac);

	return true;
}

bool OnvifNetwork::setOnvifDeviceNetWorkInfo( QString sSetIp,QString sSetMac,QString sSetGateway,QString sSetMask,QString sSetDns )
{
	if (!m_pNvpContext){
		qDebug()<<"nvp interface is NULL";
		return false;
	}
	//check ip
	if (!checkFourParam(sSetIp)){
		qDebug()<<"ip input error!";
		return false;
	}
	//check gateway
	if (!checkFourParam(sSetGateway)){
		qDebug()<<"gateway input error!";
		return false;
	}
	//check mask
	if (!checkFourParam(sSetMask)){
		qDebug()<<"mask input error!";
		return false;
	}
	//check dns
	if (!checkFourParam(sSetDns)){
		qDebug()<<"dns input error!";
		return false;
	}
	//check mac
	if (!checkMac(sSetMac)){
		qDebug()<<"mac input error!";
		return false;
	}
	stETHER_CONFIG ethCfg;
	qMemSet(&ethCfg, 0, sizeof(stETHER_CONFIG));
	int ret = m_pNvpContext->GetNetworkInterface(&m_nvpArguments, (lpNVP_ETHER_CONFIG)&ethCfg);
	if (ret){
		qDebug()<<"get network info fail";
		return false;
	}

	NVP_IP_INIT_FROM_STRING(ethCfg.ip, sSetIp.toLatin1().data());
	NVP_IP_INIT_FROM_STRING(ethCfg.gateway, sSetGateway.toLatin1().data());
	NVP_IP_INIT_FROM_STRING(ethCfg.netmask, sSetMask.toLatin1().data());
	NVP_IP_INIT_FROM_STRING(ethCfg.dns1, sSetDns.toLatin1().data());
	NVP_MAC_INIT_FROM_STRING(ethCfg.mac, sSetMac.toLatin1().data());

	ret = m_pNvpContext->SetNetworkInterface(&m_nvpArguments, (lpNVP_ETHER_CONFIG)&ethCfg);
	if (ret){
		qDebug()<<"set network config fail";
		return false;
	}
	return true;
}

QString OnvifNetwork::getOnvifDeviceEncoderInfo()
{
	QString reslut;
	if (!m_pNvpContext){
		qDebug()<<"nvp interface is NULL";
		return reslut;
	}
	qMemSet(&m_stVencConfigs, 0, sizeof(stNVP_VENC_CONFIGS));
	int ret = m_pNvpContext->GetVideoEncoderConfigs(&m_nvpArguments, &m_stVencConfigs);
	if (ret){
		qDebug()<<"get video encoder configs fail";
		return reslut;
	}
	m_nConfigNum = m_stVencConfigs.nr;
	QDomDocument doc;
	QDomElement root = doc.createElement("OnvifStreamEncoderInfo");
	root.setAttribute("itemNum", m_stVencConfigs.nr);
	doc.appendChild(root);
	for (int index = 0; index < m_stVencConfigs.nr; index++){
		QDomElement streamitem = doc.createElement("StreamItem");
		streamitem.setAttribute("index", index);
		streamitem.setAttribute("width", m_stVencConfigs.entry[index].width);
		streamitem.setAttribute("height", m_stVencConfigs.entry[index].height);
		streamitem.setAttribute("enc_fps", m_stVencConfigs.entry[index].enc_fps);
		streamitem.setAttribute("enc_bps", m_stVencConfigs.entry[index].enc_bps);
		streamitem.setAttribute("codeFormat", m_stVencConfigs.entry[index].enc_type);
		streamitem.setAttribute("enc_interval", m_stVencConfigs.entry[index].enc_interval);
		streamitem.setAttribute("enc_profile", m_stVencConfigs.entry[index].enc_profile);

		m_stVencOptions[index].index = index;
		qstrcpy(m_stVencOptions[index].token, m_stVencConfigs.entry[index].token);
		qstrcpy(m_stVencOptions[index].enc_token, m_stVencConfigs.entry[index].enc_token);
		ret = m_pNvpContext->GetVideoEncoderConfigOption(&m_nvpArguments, &m_stVencOptions[index]);
		if (ret){
			qDebug()<<"get "<<m_stVencConfigs.entry[index].token<<" options fail";
			root.appendChild(streamitem);
			continue;
		}
		//add options node
		QDomElement item = doc.createElement("EncodeOption");
		//add quality node
		QDomElement option = doc.createElement("enc_quality");
		option.setAttribute("min", m_stVencOptions[index].enc_quality.min);
		option.setAttribute("max", m_stVencOptions[index].enc_quality.max);
		item.appendChild(option);
		//add fps node
		option = doc.createElement("enc_fps");
		option.setAttribute("min", m_stVencOptions[index].enc_fps.min);
		option.setAttribute("max", m_stVencOptions[index].enc_fps.max);
		item.appendChild(option);
		//add bps node
		option = doc.createElement("enc_bps");
		option.setAttribute("min", m_stVencOptions[index].enc_bps.min);
		option.setAttribute("max", m_stVencOptions[index].enc_bps.max);
		item.appendChild(option);
		//add gov node
		option = doc.createElement("enc_gov");
		option.setAttribute("min", m_stVencOptions[index].enc_gov.min);
		option.setAttribute("max", m_stVencOptions[index].enc_gov.max);
		item.appendChild(option);
		//add interval node
		option = doc.createElement("enc_interval");
		option.setAttribute("min", m_stVencOptions[index].enc_interval.min);
		option.setAttribute("max", m_stVencOptions[index].enc_interval.max);
		item.appendChild(option);
		//add resolution node
		option = doc.createElement("resolution");
		option.setAttribute("itemNum", m_stVencOptions[index].resolution_nr);
		for (int souNum = 0; souNum < m_stVencOptions[index].resolution_nr; souNum++){
			QDomElement resolution = doc.createElement("item");
			resolution.setAttribute("width", m_stVencOptions[index].resolution[souNum].width);
			resolution.setAttribute("height", m_stVencOptions[index].resolution[souNum].height);
			option.appendChild(resolution);
		}
		item.appendChild(option);
		//add profile node
		option = doc.createElement("enc_profile");
		option.setAttribute("itemNum", m_stVencOptions[index].enc_profile_nr);
		for (int proNum = 0; proNum < m_stVencOptions[index].enc_profile_nr; proNum++){
			QDomElement profile = doc.createElement("item");
			profile.setAttribute("profile", m_stVencOptions[index].enc_profile[proNum]);
			option.appendChild(profile);
		}
		item.appendChild(option);
		streamitem.appendChild(item);
		root.appendChild(streamitem);
	}
	
	doc.save(QTextStream(&reslut), 4);
	return reslut;
}

bool OnvifNetwork::setOnvifDeviceEncoderInfo( int nIndex,int nWidth,int nHeight,QString sEnc_fps,QString sEnc_bps,QString sCodeFormat,QString sEncInterval,QString sEncProfile )
{
	//check index
	if (nIndex < 0 || nIndex >= m_nConfigNum){
		qDebug()<<"index input error!";
		return false;
	}
	//check resolution
	if (!checkResolution(nIndex, nWidth, nHeight)){
		qDebug()<<"width & height are not support!";
		return false;
	}
	//check fps
	int fps = sEnc_fps.toInt();
	if (fps < m_stVencOptions[nIndex].enc_fps.min || fps > m_stVencOptions[nIndex].enc_fps.max){
		qDebug()<<"fps input error!";
		return false;
	}
	//check bps
	int bps = sEnc_bps.toInt();
	if (bps < m_stVencOptions[nIndex].enc_bps.min || bps > m_stVencOptions[nIndex].enc_bps.max){
		qDebug()<<"bps input error!";
		return false;
	}
	//check code format
	NVP_VENC_TYPE vencType = (NVP_VENC_TYPE)sCodeFormat.toInt();
	if (NVP_VENC_JPEG != vencType && NVP_VENC_MPEG4 != vencType && NVP_VENC_H264 != vencType){
		qDebug()<<"code format input error!";
		return false;
	}
	//check key interval
	int gov = sEncInterval.toInt();
	if (gov < m_stVencOptions[nIndex].enc_gov.min || gov > m_stVencOptions[nIndex].enc_gov.max){
		qDebug()<<"interval input error!";
		return false;
	}
	//check profile
	int profile = sEncProfile.toInt();
	if (!checkProfile(nIndex, profile)){
		qDebug()<<"profile input error!";
		return false;
	}
	//fill config data
	stNVP_VENC_CONFIG vencConfig;
	qMemCopy(&vencConfig, &m_stVencConfigs.entry[nIndex], sizeof(stNVP_VENC_CONFIG));

	vencConfig.index = nIndex;
	qstrcpy(vencConfig.name, m_stVencConfigs.entry[nIndex].name);
	qstrcpy(vencConfig.token, m_stVencConfigs.entry[nIndex].token);
	qstrcpy(vencConfig.enc_name, m_stVencConfigs.entry[nIndex].enc_name);
	qstrcpy(vencConfig.enc_token, m_stVencConfigs.entry[nIndex].enc_token);
	vencConfig.width = nWidth;
	vencConfig.height = nHeight;
	vencConfig.enc_type = (NVP_VENC_TYPE)vencType;
	vencConfig.enc_fps = fps;
	vencConfig.enc_bps = bps;
	vencConfig.enc_gov = gov;
	vencConfig.enc_profile = profile;
	//set data to remote
	int ret = m_pNvpContext->SetVideoEncoderConfig(&m_nvpArguments, &vencConfig);
	if (ret){
		qDebug()<<"set video encoder config fail";
		return false;
	}
	return true;
}

bool OnvifNetwork::checkResolution( int index, int width, int height )
{
	for (int num = 0; num < m_stVencOptions[index].resolution_nr; num++){
		if (width == m_stVencOptions[index].resolution[num].width && height == m_stVencOptions[index].resolution[num].height){
			return true;
		}
	}
	return false;
}

bool OnvifNetwork::checkProfile( int index, int profile )
{
	for (int num = 0; num < m_stVencOptions[index].enc_profile_nr; num++){
		if (profile == m_stVencOptions[index].enc_profile[num]){
			return true;
		}
	}
	return false;
}

bool OnvifNetwork::checkFourParam( QString para )
{
	//check ip formate like *.*.*.*
	QRegExp rx("^(?!^0{1,3}(\\.0{1,3}){3}$)(?!^255(\\.255){3}$)((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)($|(?!\\.$)\\.)){4}$");
	return para.contains(rx);
}

bool OnvifNetwork::checkMac( QString mac )
{
	QRegExp rx("^[A-F\\d]{2}:[A-F\\d]{2}:[A-F\\d]{2}:[A-F\\d]{2}:[A-F\\d]{2}:[A-F\\d]{2}$");
	mac = mac.toUpper();
	return mac.contains(rx);
}
