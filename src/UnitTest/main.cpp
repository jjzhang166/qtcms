#include <QtGui/QApplication>
#include <QtTest/QTest>
#include "unittest.h"
#include "group_test.h"
#include "area_test.h"
#include "Device_Test.h"
#include "Channel_Test.h"
#include "dvrsearchtest.h"
#include "HiChipTest.h"
#include "RemotePreviewTest.h"
#include "DisksSettingTest.h"
#include "SetRecordTimeTest.h"
#include "RecorderTest.h"
#include "IRecorder.h"
#include "LocalPlayerSynPlaybackTest.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

// 	UnitTest tc;
// 	group_test my_group_test;
// 	area_test my_area_test;
// 	Device_Test my_devive_test;
// 	Channel_Test my_channel_test;
// 	DvrSearchTest test;
// 	HiChipUnitTest test1;
// 	RemotePreviewTest remoteViewTest;
// 	DisksSettingTest my_diskssetting_test;
//	SetRecordTimeTest my_SetRecordTime_Test;
//	RecorderTest my_Recorder_Test;
    LocalPlayerSynPlaybackTest my_localPlayback_Test;
// 	QTest::qExec(&my_devive_test,argc,argv);
// 	QTest::qExec(&my_area_test,argc,argv);
// 	QTest::qExec(&tc, argc, argv);
// 	QTest::qExec(&my_group_test,argc,argv);
// 	QTest::qExec(&my_channel_test,argc,argv);
// 	QTest::qExec(&test,argc,argv);
// 	QTest::qExec(&test1, argc, argv);
// 	QTest::qExec(&my_diskssetting_test, argc, argv);
// 	QTest::qExec(&remoteViewTest, argc, argv);
//	QTest::qExec(&my_Recorder_Test, argc, argv);
//  QTest::qExec(&my_SetRecordTime_Test,argc,argv);	
    QTest::qExec(&my_localPlayback_Test, argc,argv);
	return 0;
}
