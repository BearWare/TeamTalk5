/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qtTeamTalk/mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[176];
    char stringdata0[3475];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 12), // "serverUpdate"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 16), // "ServerProperties"
QT_MOC_LITERAL(4, 42, 7), // "srvprop"
QT_MOC_LITERAL(5, 50, 16), // "serverStatistics"
QT_MOC_LITERAL(6, 67, 16), // "ServerStatistics"
QT_MOC_LITERAL(7, 84, 5), // "stats"
QT_MOC_LITERAL(8, 90, 10), // "newChannel"
QT_MOC_LITERAL(9, 101, 7), // "Channel"
QT_MOC_LITERAL(10, 109, 4), // "chan"
QT_MOC_LITERAL(11, 114, 13), // "updateChannel"
QT_MOC_LITERAL(12, 128, 13), // "removeChannel"
QT_MOC_LITERAL(13, 142, 9), // "userLogin"
QT_MOC_LITERAL(14, 152, 4), // "User"
QT_MOC_LITERAL(15, 157, 4), // "user"
QT_MOC_LITERAL(16, 162, 10), // "userLogout"
QT_MOC_LITERAL(17, 173, 10), // "userUpdate"
QT_MOC_LITERAL(18, 184, 10), // "userJoined"
QT_MOC_LITERAL(19, 195, 9), // "channelid"
QT_MOC_LITERAL(20, 205, 8), // "userLeft"
QT_MOC_LITERAL(21, 214, 15), // "userStateChange"
QT_MOC_LITERAL(22, 230, 12), // "updateMyself"
QT_MOC_LITERAL(23, 243, 20), // "newVideoCaptureFrame"
QT_MOC_LITERAL(24, 264, 6), // "userid"
QT_MOC_LITERAL(25, 271, 9), // "stream_id"
QT_MOC_LITERAL(26, 281, 18), // "newMediaVideoFrame"
QT_MOC_LITERAL(27, 300, 16), // "newDesktopWindow"
QT_MOC_LITERAL(28, 317, 9), // "sessionid"
QT_MOC_LITERAL(29, 327, 17), // "userDesktopCursor"
QT_MOC_LITERAL(30, 345, 10), // "src_userid"
QT_MOC_LITERAL(31, 356, 12), // "DesktopInput"
QT_MOC_LITERAL(32, 369, 14), // "newTextMessage"
QT_MOC_LITERAL(33, 384, 11), // "TextMessage"
QT_MOC_LITERAL(34, 396, 7), // "textmsg"
QT_MOC_LITERAL(35, 404, 18), // "filetransferUpdate"
QT_MOC_LITERAL(36, 423, 12), // "FileTransfer"
QT_MOC_LITERAL(37, 436, 8), // "transfer"
QT_MOC_LITERAL(38, 445, 17), // "mediaStreamUpdate"
QT_MOC_LITERAL(39, 463, 13), // "MediaFileInfo"
QT_MOC_LITERAL(40, 477, 3), // "mfi"
QT_MOC_LITERAL(41, 481, 19), // "mediaPlaybackUpdate"
QT_MOC_LITERAL(42, 501, 9), // "sessionID"
QT_MOC_LITERAL(43, 511, 10), // "cmdSuccess"
QT_MOC_LITERAL(44, 522, 5), // "cmdid"
QT_MOC_LITERAL(45, 528, 8), // "cmdError"
QT_MOC_LITERAL(46, 537, 5), // "error"
QT_MOC_LITERAL(47, 543, 19), // "preferencesModified"
QT_MOC_LITERAL(48, 563, 21), // "slotClientNewInstance"
QT_MOC_LITERAL(49, 585, 7), // "checked"
QT_MOC_LITERAL(50, 593, 17), // "slotClientConnect"
QT_MOC_LITERAL(51, 611, 21), // "slotClientPreferences"
QT_MOC_LITERAL(52, 633, 14), // "slotClientExit"
QT_MOC_LITERAL(53, 648, 20), // "slotMeChangeNickname"
QT_MOC_LITERAL(54, 669, 18), // "slotMeChangeStatus"
QT_MOC_LITERAL(55, 688, 22), // "slotMeEnablePushToTalk"
QT_MOC_LITERAL(56, 711, 27), // "slotMeEnableVoiceActivation"
QT_MOC_LITERAL(57, 739, 29), // "slotMeEnableVideoTransmission"
QT_MOC_LITERAL(58, 769, 26), // "slotMeEnableDesktopSharing"
QT_MOC_LITERAL(59, 796, 28), // "slotUsersViewUserInformation"
QT_MOC_LITERAL(60, 825, 17), // "slotUsersMessages"
QT_MOC_LITERAL(61, 843, 18), // "slotUsersMuteVoice"
QT_MOC_LITERAL(62, 862, 22), // "slotUsersMuteMediaFile"
QT_MOC_LITERAL(63, 885, 15), // "slotUsersVolume"
QT_MOC_LITERAL(64, 901, 11), // "slotUsersOp"
QT_MOC_LITERAL(65, 913, 24), // "slotUsersKickFromChannel"
QT_MOC_LITERAL(66, 938, 27), // "slotUsersKickBanFromChannel"
QT_MOC_LITERAL(67, 966, 23), // "slotUsersKickFromServer"
QT_MOC_LITERAL(68, 990, 26), // "slotUsersKickBanFromServer"
QT_MOC_LITERAL(69, 1017, 29), // "slotUsersSubscriptionsUserMsg"
QT_MOC_LITERAL(70, 1047, 32), // "slotUsersSubscriptionsChannelMsg"
QT_MOC_LITERAL(71, 1080, 30), // "slotUsersSubscriptionsBCastMsg"
QT_MOC_LITERAL(72, 1111, 27), // "slotUsersSubscriptionsVoice"
QT_MOC_LITERAL(73, 1139, 27), // "slotUsersSubscriptionsVideo"
QT_MOC_LITERAL(74, 1167, 29), // "slotUsersSubscriptionsDesktop"
QT_MOC_LITERAL(75, 1197, 34), // "slotUsersSubscriptionsDesktop..."
QT_MOC_LITERAL(76, 1232, 31), // "slotUsersSubscriptionsMediaFile"
QT_MOC_LITERAL(77, 1264, 38), // "slotUsersSubscriptionsInterce..."
QT_MOC_LITERAL(78, 1303, 41), // "slotUsersSubscriptionsInterce..."
QT_MOC_LITERAL(79, 1345, 36), // "slotUsersSubscriptionsInterce..."
QT_MOC_LITERAL(80, 1382, 36), // "slotUsersSubscriptionsInterce..."
QT_MOC_LITERAL(81, 1419, 38), // "slotUsersSubscriptionsInterce..."
QT_MOC_LITERAL(82, 1458, 40), // "slotUsersSubscriptionsInterce..."
QT_MOC_LITERAL(83, 1499, 31), // "slotUsersAdvancedIncVolumeVoice"
QT_MOC_LITERAL(84, 1531, 31), // "slotUsersAdvancedDecVolumeVoice"
QT_MOC_LITERAL(85, 1563, 35), // "slotUsersAdvancedIncVolumeMed..."
QT_MOC_LITERAL(86, 1599, 35), // "slotUsersAdvancedDecVolumeMed..."
QT_MOC_LITERAL(87, 1635, 29), // "slotUsersAdvancedStoreForMove"
QT_MOC_LITERAL(88, 1665, 26), // "slotUsersAdvancedMoveUsers"
QT_MOC_LITERAL(89, 1692, 29), // "slotUsersAdvancedVoiceAllowed"
QT_MOC_LITERAL(90, 1722, 29), // "slotUsersAdvancedVideoAllowed"
QT_MOC_LITERAL(91, 1752, 31), // "slotUsersAdvancedDesktopAllowed"
QT_MOC_LITERAL(92, 1784, 33), // "slotUsersAdvancedMediaFileAll..."
QT_MOC_LITERAL(93, 1818, 21), // "slotUsersMuteVoiceAll"
QT_MOC_LITERAL(94, 1840, 25), // "slotUsersStoreAudioToDisk"
QT_MOC_LITERAL(95, 1866, 25), // "slotChannelsCreateChannel"
QT_MOC_LITERAL(96, 1892, 25), // "slotChannelsUpdateChannel"
QT_MOC_LITERAL(97, 1918, 25), // "slotChannelsDeleteChannel"
QT_MOC_LITERAL(98, 1944, 23), // "slotChannelsJoinChannel"
QT_MOC_LITERAL(99, 1968, 27), // "slotChannelsViewChannelInfo"
QT_MOC_LITERAL(100, 1996, 20), // "slotChannelsListBans"
QT_MOC_LITERAL(101, 2017, 27), // "slotChannelsStreamMediaFile"
QT_MOC_LITERAL(102, 2045, 22), // "slotChannelsUploadFile"
QT_MOC_LITERAL(103, 2068, 24), // "slotChannelsDownloadFile"
QT_MOC_LITERAL(104, 2093, 22), // "slotChannelsDeleteFile"
QT_MOC_LITERAL(105, 2116, 22), // "slotServerUserAccounts"
QT_MOC_LITERAL(106, 2139, 21), // "slotServerBannedUsers"
QT_MOC_LITERAL(107, 2161, 21), // "slotServerOnlineUsers"
QT_MOC_LITERAL(108, 2183, 26), // "slotServerBroadcastMessage"
QT_MOC_LITERAL(109, 2210, 26), // "slotServerServerProperties"
QT_MOC_LITERAL(110, 2237, 27), // "slotServerSaveConfiguration"
QT_MOC_LITERAL(111, 2265, 26), // "slotServerServerStatistics"
QT_MOC_LITERAL(112, 2292, 14), // "slotHelpManual"
QT_MOC_LITERAL(113, 2307, 24), // "slotHelpResetPreferences"
QT_MOC_LITERAL(114, 2332, 21), // "slotHelpVisitBearWare"
QT_MOC_LITERAL(115, 2354, 22), // "slotHelpVisitChangeLog"
QT_MOC_LITERAL(116, 2377, 13), // "slotHelpAbout"
QT_MOC_LITERAL(117, 2391, 19), // "slotConnectToLatest"
QT_MOC_LITERAL(118, 2411, 4), // "mute"
QT_MOC_LITERAL(119, 2416, 6), // "chanid"
QT_MOC_LITERAL(120, 2423, 13), // "slotUsersKick"
QT_MOC_LITERAL(121, 2437, 16), // "slotUsersKickBan"
QT_MOC_LITERAL(122, 2454, 24), // "slotTreeSelectionChanged"
QT_MOC_LITERAL(123, 2479, 19), // "slotTreeContextMenu"
QT_MOC_LITERAL(124, 2499, 3), // "pos"
QT_MOC_LITERAL(125, 2503, 12), // "slotUpdateUI"
QT_MOC_LITERAL(126, 2516, 20), // "slotUpdateVideoTabUI"
QT_MOC_LITERAL(127, 2537, 22), // "slotUpdateDesktopTabUI"
QT_MOC_LITERAL(128, 2560, 15), // "slotUploadFiles"
QT_MOC_LITERAL(129, 2576, 5), // "files"
QT_MOC_LITERAL(130, 2582, 22), // "slotSendChannelMessage"
QT_MOC_LITERAL(131, 2605, 21), // "slotUserDoubleClicked"
QT_MOC_LITERAL(132, 2627, 24), // "slotChannelDoubleClicked"
QT_MOC_LITERAL(133, 2652, 18), // "slotNewTextMessage"
QT_MOC_LITERAL(134, 2671, 24), // "slotNewMyselfTextMessage"
QT_MOC_LITERAL(135, 2696, 21), // "slotTextMessageClosed"
QT_MOC_LITERAL(136, 2718, 24), // "slotTransmitUsersChanged"
QT_MOC_LITERAL(137, 2743, 21), // "QMap<int,StreamTypes>"
QT_MOC_LITERAL(138, 2765, 17), // "slotChannelUpdate"
QT_MOC_LITERAL(139, 2783, 13), // "slotInitVideo"
QT_MOC_LITERAL(140, 2797, 16), // "slotAddUserVideo"
QT_MOC_LITERAL(141, 2814, 19), // "slotRemoveUserVideo"
QT_MOC_LITERAL(142, 2834, 19), // "slotDetachUserVideo"
QT_MOC_LITERAL(143, 2854, 19), // "slotNewUserVideoDlg"
QT_MOC_LITERAL(144, 2874, 4), // "size"
QT_MOC_LITERAL(145, 2879, 23), // "slotUserVideoDlgClosing"
QT_MOC_LITERAL(146, 2903, 18), // "slotAddUserDesktop"
QT_MOC_LITERAL(147, 2922, 21), // "slotRemoveUserDesktop"
QT_MOC_LITERAL(148, 2944, 21), // "slotAccessUserDesktop"
QT_MOC_LITERAL(149, 2966, 6), // "enable"
QT_MOC_LITERAL(150, 2973, 21), // "slotDetachUserDesktop"
QT_MOC_LITERAL(151, 2995, 25), // "slotUserDesktopDlgClosing"
QT_MOC_LITERAL(152, 3021, 12), // "slotUserJoin"
QT_MOC_LITERAL(153, 3034, 12), // "slotUserLeft"
QT_MOC_LITERAL(154, 3047, 14), // "slotUserUpdate"
QT_MOC_LITERAL(155, 3062, 22), // "slotEnableQuestionMode"
QT_MOC_LITERAL(156, 3085, 20), // "slotUpdateVideoCount"
QT_MOC_LITERAL(157, 3106, 5), // "count"
QT_MOC_LITERAL(158, 3112, 22), // "slotUpdateDesktopCount"
QT_MOC_LITERAL(159, 3135, 23), // "slotMasterVolumeChanged"
QT_MOC_LITERAL(160, 3159, 5), // "value"
QT_MOC_LITERAL(161, 3165, 25), // "slotMicrophoneGainChanged"
QT_MOC_LITERAL(162, 3191, 31), // "slotVoiceActivationLevelChanged"
QT_MOC_LITERAL(163, 3223, 18), // "slotTrayIconChange"
QT_MOC_LITERAL(164, 3242, 33), // "QSystemTrayIcon::ActivationRe..."
QT_MOC_LITERAL(165, 3276, 6), // "reason"
QT_MOC_LITERAL(166, 3283, 14), // "slotLoadTTFile"
QT_MOC_LITERAL(167, 3298, 8), // "filepath"
QT_MOC_LITERAL(168, 3307, 23), // "slotSoftwareUpdateReply"
QT_MOC_LITERAL(169, 3331, 14), // "QNetworkReply*"
QT_MOC_LITERAL(170, 3346, 5), // "reply"
QT_MOC_LITERAL(171, 3352, 21), // "slotBearWareAuthReply"
QT_MOC_LITERAL(172, 3374, 24), // "slotClosedOnlineUsersDlg"
QT_MOC_LITERAL(173, 3399, 24), // "slotClosedServerStatsDlg"
QT_MOC_LITERAL(174, 3424, 25), // "slotClosedUserAccountsDlg"
QT_MOC_LITERAL(175, 3450, 24) // "slotClosedBannedUsersDlg"

    },
    "MainWindow\0serverUpdate\0\0ServerProperties\0"
    "srvprop\0serverStatistics\0ServerStatistics\0"
    "stats\0newChannel\0Channel\0chan\0"
    "updateChannel\0removeChannel\0userLogin\0"
    "User\0user\0userLogout\0userUpdate\0"
    "userJoined\0channelid\0userLeft\0"
    "userStateChange\0updateMyself\0"
    "newVideoCaptureFrame\0userid\0stream_id\0"
    "newMediaVideoFrame\0newDesktopWindow\0"
    "sessionid\0userDesktopCursor\0src_userid\0"
    "DesktopInput\0newTextMessage\0TextMessage\0"
    "textmsg\0filetransferUpdate\0FileTransfer\0"
    "transfer\0mediaStreamUpdate\0MediaFileInfo\0"
    "mfi\0mediaPlaybackUpdate\0sessionID\0"
    "cmdSuccess\0cmdid\0cmdError\0error\0"
    "preferencesModified\0slotClientNewInstance\0"
    "checked\0slotClientConnect\0"
    "slotClientPreferences\0slotClientExit\0"
    "slotMeChangeNickname\0slotMeChangeStatus\0"
    "slotMeEnablePushToTalk\0"
    "slotMeEnableVoiceActivation\0"
    "slotMeEnableVideoTransmission\0"
    "slotMeEnableDesktopSharing\0"
    "slotUsersViewUserInformation\0"
    "slotUsersMessages\0slotUsersMuteVoice\0"
    "slotUsersMuteMediaFile\0slotUsersVolume\0"
    "slotUsersOp\0slotUsersKickFromChannel\0"
    "slotUsersKickBanFromChannel\0"
    "slotUsersKickFromServer\0"
    "slotUsersKickBanFromServer\0"
    "slotUsersSubscriptionsUserMsg\0"
    "slotUsersSubscriptionsChannelMsg\0"
    "slotUsersSubscriptionsBCastMsg\0"
    "slotUsersSubscriptionsVoice\0"
    "slotUsersSubscriptionsVideo\0"
    "slotUsersSubscriptionsDesktop\0"
    "slotUsersSubscriptionsDesktopInput\0"
    "slotUsersSubscriptionsMediaFile\0"
    "slotUsersSubscriptionsInterceptUserMsg\0"
    "slotUsersSubscriptionsInterceptChannelMsg\0"
    "slotUsersSubscriptionsInterceptVoice\0"
    "slotUsersSubscriptionsInterceptVideo\0"
    "slotUsersSubscriptionsInterceptDesktop\0"
    "slotUsersSubscriptionsInterceptMediaFile\0"
    "slotUsersAdvancedIncVolumeVoice\0"
    "slotUsersAdvancedDecVolumeVoice\0"
    "slotUsersAdvancedIncVolumeMediaFile\0"
    "slotUsersAdvancedDecVolumeMediaFile\0"
    "slotUsersAdvancedStoreForMove\0"
    "slotUsersAdvancedMoveUsers\0"
    "slotUsersAdvancedVoiceAllowed\0"
    "slotUsersAdvancedVideoAllowed\0"
    "slotUsersAdvancedDesktopAllowed\0"
    "slotUsersAdvancedMediaFileAllowed\0"
    "slotUsersMuteVoiceAll\0slotUsersStoreAudioToDisk\0"
    "slotChannelsCreateChannel\0"
    "slotChannelsUpdateChannel\0"
    "slotChannelsDeleteChannel\0"
    "slotChannelsJoinChannel\0"
    "slotChannelsViewChannelInfo\0"
    "slotChannelsListBans\0slotChannelsStreamMediaFile\0"
    "slotChannelsUploadFile\0slotChannelsDownloadFile\0"
    "slotChannelsDeleteFile\0slotServerUserAccounts\0"
    "slotServerBannedUsers\0slotServerOnlineUsers\0"
    "slotServerBroadcastMessage\0"
    "slotServerServerProperties\0"
    "slotServerSaveConfiguration\0"
    "slotServerServerStatistics\0slotHelpManual\0"
    "slotHelpResetPreferences\0slotHelpVisitBearWare\0"
    "slotHelpVisitChangeLog\0slotHelpAbout\0"
    "slotConnectToLatest\0mute\0chanid\0"
    "slotUsersKick\0slotUsersKickBan\0"
    "slotTreeSelectionChanged\0slotTreeContextMenu\0"
    "pos\0slotUpdateUI\0slotUpdateVideoTabUI\0"
    "slotUpdateDesktopTabUI\0slotUploadFiles\0"
    "files\0slotSendChannelMessage\0"
    "slotUserDoubleClicked\0slotChannelDoubleClicked\0"
    "slotNewTextMessage\0slotNewMyselfTextMessage\0"
    "slotTextMessageClosed\0slotTransmitUsersChanged\0"
    "QMap<int,StreamTypes>\0slotChannelUpdate\0"
    "slotInitVideo\0slotAddUserVideo\0"
    "slotRemoveUserVideo\0slotDetachUserVideo\0"
    "slotNewUserVideoDlg\0size\0"
    "slotUserVideoDlgClosing\0slotAddUserDesktop\0"
    "slotRemoveUserDesktop\0slotAccessUserDesktop\0"
    "enable\0slotDetachUserDesktop\0"
    "slotUserDesktopDlgClosing\0slotUserJoin\0"
    "slotUserLeft\0slotUserUpdate\0"
    "slotEnableQuestionMode\0slotUpdateVideoCount\0"
    "count\0slotUpdateDesktopCount\0"
    "slotMasterVolumeChanged\0value\0"
    "slotMicrophoneGainChanged\0"
    "slotVoiceActivationLevelChanged\0"
    "slotTrayIconChange\0QSystemTrayIcon::ActivationReason\0"
    "reason\0slotLoadTTFile\0filepath\0"
    "slotSoftwareUpdateReply\0QNetworkReply*\0"
    "reply\0slotBearWareAuthReply\0"
    "slotClosedOnlineUsersDlg\0"
    "slotClosedServerStatsDlg\0"
    "slotClosedUserAccountsDlg\0"
    "slotClosedBannedUsersDlg"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
     208,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      23,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1, 1054,    2, 0x06 /* Public */,
       5,    1, 1057,    2, 0x06 /* Public */,
       8,    1, 1060,    2, 0x06 /* Public */,
      11,    1, 1063,    2, 0x06 /* Public */,
      12,    1, 1066,    2, 0x06 /* Public */,
      13,    1, 1069,    2, 0x06 /* Public */,
      16,    1, 1072,    2, 0x06 /* Public */,
      17,    1, 1075,    2, 0x06 /* Public */,
      18,    2, 1078,    2, 0x06 /* Public */,
      20,    2, 1083,    2, 0x06 /* Public */,
      21,    1, 1088,    2, 0x06 /* Public */,
      22,    0, 1091,    2, 0x06 /* Public */,
      23,    2, 1092,    2, 0x06 /* Public */,
      26,    2, 1097,    2, 0x06 /* Public */,
      27,    2, 1102,    2, 0x06 /* Public */,
      29,    2, 1107,    2, 0x06 /* Public */,
      32,    1, 1112,    2, 0x06 /* Public */,
      35,    1, 1115,    2, 0x06 /* Public */,
      38,    1, 1118,    2, 0x06 /* Public */,
      41,    2, 1121,    2, 0x06 /* Public */,
      43,    1, 1126,    2, 0x06 /* Public */,
      45,    2, 1129,    2, 0x06 /* Public */,
      47,    0, 1134,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      48,    1, 1135,    2, 0x08 /* Private */,
      48,    0, 1138,    2, 0x28 /* Private | MethodCloned */,
      50,    1, 1139,    2, 0x08 /* Private */,
      50,    0, 1142,    2, 0x28 /* Private | MethodCloned */,
      51,    1, 1143,    2, 0x08 /* Private */,
      51,    0, 1146,    2, 0x28 /* Private | MethodCloned */,
      52,    1, 1147,    2, 0x08 /* Private */,
      52,    0, 1150,    2, 0x28 /* Private | MethodCloned */,
      53,    1, 1151,    2, 0x08 /* Private */,
      53,    0, 1154,    2, 0x28 /* Private | MethodCloned */,
      54,    1, 1155,    2, 0x08 /* Private */,
      54,    0, 1158,    2, 0x28 /* Private | MethodCloned */,
      55,    1, 1159,    2, 0x08 /* Private */,
      55,    0, 1162,    2, 0x28 /* Private | MethodCloned */,
      56,    1, 1163,    2, 0x08 /* Private */,
      56,    0, 1166,    2, 0x28 /* Private | MethodCloned */,
      57,    1, 1167,    2, 0x08 /* Private */,
      57,    0, 1170,    2, 0x28 /* Private | MethodCloned */,
      58,    1, 1171,    2, 0x08 /* Private */,
      58,    0, 1174,    2, 0x28 /* Private | MethodCloned */,
      59,    1, 1175,    2, 0x08 /* Private */,
      59,    0, 1178,    2, 0x28 /* Private | MethodCloned */,
      60,    1, 1179,    2, 0x08 /* Private */,
      60,    0, 1182,    2, 0x28 /* Private | MethodCloned */,
      61,    1, 1183,    2, 0x08 /* Private */,
      61,    0, 1186,    2, 0x28 /* Private | MethodCloned */,
      62,    1, 1187,    2, 0x08 /* Private */,
      62,    0, 1190,    2, 0x28 /* Private | MethodCloned */,
      63,    1, 1191,    2, 0x08 /* Private */,
      63,    0, 1194,    2, 0x28 /* Private | MethodCloned */,
      64,    1, 1195,    2, 0x08 /* Private */,
      64,    0, 1198,    2, 0x28 /* Private | MethodCloned */,
      65,    1, 1199,    2, 0x08 /* Private */,
      65,    0, 1202,    2, 0x28 /* Private | MethodCloned */,
      66,    1, 1203,    2, 0x08 /* Private */,
      66,    0, 1206,    2, 0x28 /* Private | MethodCloned */,
      67,    1, 1207,    2, 0x08 /* Private */,
      67,    0, 1210,    2, 0x28 /* Private | MethodCloned */,
      68,    1, 1211,    2, 0x08 /* Private */,
      68,    0, 1214,    2, 0x28 /* Private | MethodCloned */,
      69,    1, 1215,    2, 0x08 /* Private */,
      69,    0, 1218,    2, 0x28 /* Private | MethodCloned */,
      70,    1, 1219,    2, 0x08 /* Private */,
      70,    0, 1222,    2, 0x28 /* Private | MethodCloned */,
      71,    1, 1223,    2, 0x08 /* Private */,
      71,    0, 1226,    2, 0x28 /* Private | MethodCloned */,
      72,    1, 1227,    2, 0x08 /* Private */,
      72,    0, 1230,    2, 0x28 /* Private | MethodCloned */,
      73,    1, 1231,    2, 0x08 /* Private */,
      73,    0, 1234,    2, 0x28 /* Private | MethodCloned */,
      74,    1, 1235,    2, 0x08 /* Private */,
      74,    0, 1238,    2, 0x28 /* Private | MethodCloned */,
      75,    1, 1239,    2, 0x08 /* Private */,
      75,    0, 1242,    2, 0x28 /* Private | MethodCloned */,
      76,    1, 1243,    2, 0x08 /* Private */,
      76,    0, 1246,    2, 0x28 /* Private | MethodCloned */,
      77,    1, 1247,    2, 0x08 /* Private */,
      77,    0, 1250,    2, 0x28 /* Private | MethodCloned */,
      78,    1, 1251,    2, 0x08 /* Private */,
      78,    0, 1254,    2, 0x28 /* Private | MethodCloned */,
      79,    1, 1255,    2, 0x08 /* Private */,
      79,    0, 1258,    2, 0x28 /* Private | MethodCloned */,
      80,    1, 1259,    2, 0x08 /* Private */,
      80,    0, 1262,    2, 0x28 /* Private | MethodCloned */,
      81,    1, 1263,    2, 0x08 /* Private */,
      81,    0, 1266,    2, 0x28 /* Private | MethodCloned */,
      82,    1, 1267,    2, 0x08 /* Private */,
      82,    0, 1270,    2, 0x28 /* Private | MethodCloned */,
      83,    0, 1271,    2, 0x08 /* Private */,
      84,    0, 1272,    2, 0x08 /* Private */,
      85,    0, 1273,    2, 0x08 /* Private */,
      86,    0, 1274,    2, 0x08 /* Private */,
      87,    0, 1275,    2, 0x08 /* Private */,
      88,    0, 1276,    2, 0x08 /* Private */,
      89,    1, 1277,    2, 0x08 /* Private */,
      89,    0, 1280,    2, 0x28 /* Private | MethodCloned */,
      90,    1, 1281,    2, 0x08 /* Private */,
      90,    0, 1284,    2, 0x28 /* Private | MethodCloned */,
      91,    1, 1285,    2, 0x08 /* Private */,
      91,    0, 1288,    2, 0x28 /* Private | MethodCloned */,
      92,    1, 1289,    2, 0x08 /* Private */,
      92,    0, 1292,    2, 0x28 /* Private | MethodCloned */,
      93,    1, 1293,    2, 0x08 /* Private */,
      93,    0, 1296,    2, 0x28 /* Private | MethodCloned */,
      94,    1, 1297,    2, 0x08 /* Private */,
      94,    0, 1300,    2, 0x28 /* Private | MethodCloned */,
      95,    1, 1301,    2, 0x08 /* Private */,
      95,    0, 1304,    2, 0x28 /* Private | MethodCloned */,
      96,    1, 1305,    2, 0x08 /* Private */,
      96,    0, 1308,    2, 0x28 /* Private | MethodCloned */,
      97,    1, 1309,    2, 0x08 /* Private */,
      97,    0, 1312,    2, 0x28 /* Private | MethodCloned */,
      98,    1, 1313,    2, 0x08 /* Private */,
      98,    0, 1316,    2, 0x28 /* Private | MethodCloned */,
      99,    1, 1317,    2, 0x08 /* Private */,
      99,    0, 1320,    2, 0x28 /* Private | MethodCloned */,
     100,    1, 1321,    2, 0x08 /* Private */,
     100,    0, 1324,    2, 0x28 /* Private | MethodCloned */,
     101,    1, 1325,    2, 0x08 /* Private */,
     101,    0, 1328,    2, 0x28 /* Private | MethodCloned */,
     102,    1, 1329,    2, 0x08 /* Private */,
     102,    0, 1332,    2, 0x28 /* Private | MethodCloned */,
     103,    1, 1333,    2, 0x08 /* Private */,
     103,    0, 1336,    2, 0x28 /* Private | MethodCloned */,
     104,    1, 1337,    2, 0x08 /* Private */,
     104,    0, 1340,    2, 0x28 /* Private | MethodCloned */,
     105,    1, 1341,    2, 0x08 /* Private */,
     105,    0, 1344,    2, 0x28 /* Private | MethodCloned */,
     106,    1, 1345,    2, 0x08 /* Private */,
     106,    0, 1348,    2, 0x28 /* Private | MethodCloned */,
     107,    1, 1349,    2, 0x08 /* Private */,
     107,    0, 1352,    2, 0x28 /* Private | MethodCloned */,
     108,    1, 1353,    2, 0x08 /* Private */,
     108,    0, 1356,    2, 0x28 /* Private | MethodCloned */,
     109,    1, 1357,    2, 0x08 /* Private */,
     109,    0, 1360,    2, 0x28 /* Private | MethodCloned */,
     110,    1, 1361,    2, 0x08 /* Private */,
     110,    0, 1364,    2, 0x28 /* Private | MethodCloned */,
     111,    1, 1365,    2, 0x08 /* Private */,
     111,    0, 1368,    2, 0x28 /* Private | MethodCloned */,
     112,    1, 1369,    2, 0x08 /* Private */,
     112,    0, 1372,    2, 0x28 /* Private | MethodCloned */,
     113,    1, 1373,    2, 0x08 /* Private */,
     113,    0, 1376,    2, 0x28 /* Private | MethodCloned */,
     114,    1, 1377,    2, 0x08 /* Private */,
     114,    0, 1380,    2, 0x28 /* Private | MethodCloned */,
     115,    1, 1381,    2, 0x08 /* Private */,
     115,    0, 1384,    2, 0x28 /* Private | MethodCloned */,
     116,    1, 1385,    2, 0x08 /* Private */,
     116,    0, 1388,    2, 0x28 /* Private | MethodCloned */,
     117,    0, 1389,    2, 0x08 /* Private */,
      59,    1, 1390,    2, 0x08 /* Private */,
      60,    1, 1393,    2, 0x08 /* Private */,
      61,    2, 1396,    2, 0x08 /* Private */,
      62,    2, 1401,    2, 0x08 /* Private */,
      63,    1, 1406,    2, 0x08 /* Private */,
      64,    2, 1409,    2, 0x08 /* Private */,
     120,    2, 1414,    2, 0x08 /* Private */,
     121,    2, 1419,    2, 0x08 /* Private */,
     122,    0, 1424,    2, 0x08 /* Private */,
     123,    1, 1425,    2, 0x08 /* Private */,
     125,    0, 1428,    2, 0x08 /* Private */,
     126,    0, 1429,    2, 0x08 /* Private */,
     127,    0, 1430,    2, 0x08 /* Private */,
     128,    1, 1431,    2, 0x08 /* Private */,
     130,    0, 1434,    2, 0x08 /* Private */,
     131,    1, 1435,    2, 0x08 /* Private */,
     132,    1, 1438,    2, 0x08 /* Private */,
     133,    1, 1441,    2, 0x08 /* Private */,
     134,    1, 1444,    2, 0x08 /* Private */,
     135,    1, 1447,    2, 0x08 /* Private */,
     136,    2, 1450,    2, 0x08 /* Private */,
     138,    1, 1455,    2, 0x08 /* Private */,
     139,    0, 1458,    2, 0x08 /* Private */,
     140,    0, 1459,    2, 0x08 /* Private */,
     141,    0, 1460,    2, 0x08 /* Private */,
     141,    1, 1461,    2, 0x08 /* Private */,
     142,    1, 1464,    2, 0x08 /* Private */,
     142,    0, 1467,    2, 0x28 /* Private | MethodCloned */,
     143,    2, 1468,    2, 0x08 /* Private */,
     145,    1, 1473,    2, 0x08 /* Private */,
     146,    0, 1476,    2, 0x08 /* Private */,
     147,    0, 1477,    2, 0x08 /* Private */,
     147,    1, 1478,    2, 0x08 /* Private */,
     148,    1, 1481,    2, 0x08 /* Private */,
     150,    0, 1484,    2, 0x08 /* Private */,
     150,    2, 1485,    2, 0x08 /* Private */,
     151,    1, 1490,    2, 0x08 /* Private */,
     152,    2, 1493,    2, 0x08 /* Private */,
     153,    2, 1498,    2, 0x08 /* Private */,
     154,    1, 1503,    2, 0x08 /* Private */,
     155,    1, 1506,    2, 0x08 /* Private */,
     156,    1, 1509,    2, 0x08 /* Private */,
     158,    1, 1512,    2, 0x08 /* Private */,
     159,    1, 1515,    2, 0x08 /* Private */,
     161,    1, 1518,    2, 0x08 /* Private */,
     162,    1, 1521,    2, 0x08 /* Private */,
     163,    1, 1524,    2, 0x08 /* Private */,
     166,    1, 1527,    2, 0x08 /* Private */,
     168,    1, 1530,    2, 0x08 /* Private */,
     171,    1, 1533,    2, 0x08 /* Private */,
     172,    1, 1536,    2, 0x08 /* Private */,
     173,    1, 1539,    2, 0x08 /* Private */,
     174,    1, 1542,    2, 0x08 /* Private */,
     175,    1, 1545,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 14,   19,   15,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 14,   19,   15,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   24,   25,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   24,   25,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   24,   28,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 31,   30,    2,
    QMetaType::Void, 0x80000000 | 33,   34,
    QMetaType::Void, 0x80000000 | 36,   37,
    QMetaType::Void, 0x80000000 | 39,   40,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 39,   42,   40,
    QMetaType::Void, QMetaType::Int,   44,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   46,   44,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,   24,  118,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,   24,  118,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   24,  119,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   24,  119,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   24,  119,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPoint,  124,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QStringList,  129,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void, 0x80000000 | 33,   34,
    QMetaType::Void, 0x80000000 | 33,   34,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 137,   19,    2,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::QSize,   24,  144,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Bool,  149,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::QSize,   24,  144,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 14,   19,   15,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 14,   19,   15,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, QMetaType::Bool,   49,
    QMetaType::Void, QMetaType::Int,  157,
    QMetaType::Void, QMetaType::Int,  157,
    QMetaType::Void, QMetaType::Int,  160,
    QMetaType::Void, QMetaType::Int,  160,
    QMetaType::Void, QMetaType::Int,  160,
    QMetaType::Void, 0x80000000 | 164,  165,
    QMetaType::Void, QMetaType::QString,  167,
    QMetaType::Void, 0x80000000 | 169,  170,
    QMetaType::Void, 0x80000000 | 169,  170,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->serverUpdate((*reinterpret_cast< const ServerProperties(*)>(_a[1]))); break;
        case 1: _t->serverStatistics((*reinterpret_cast< const ServerStatistics(*)>(_a[1]))); break;
        case 2: _t->newChannel((*reinterpret_cast< const Channel(*)>(_a[1]))); break;
        case 3: _t->updateChannel((*reinterpret_cast< const Channel(*)>(_a[1]))); break;
        case 4: _t->removeChannel((*reinterpret_cast< const Channel(*)>(_a[1]))); break;
        case 5: _t->userLogin((*reinterpret_cast< const User(*)>(_a[1]))); break;
        case 6: _t->userLogout((*reinterpret_cast< const User(*)>(_a[1]))); break;
        case 7: _t->userUpdate((*reinterpret_cast< const User(*)>(_a[1]))); break;
        case 8: _t->userJoined((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const User(*)>(_a[2]))); break;
        case 9: _t->userLeft((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const User(*)>(_a[2]))); break;
        case 10: _t->userStateChange((*reinterpret_cast< const User(*)>(_a[1]))); break;
        case 11: _t->updateMyself(); break;
        case 12: _t->newVideoCaptureFrame((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 13: _t->newMediaVideoFrame((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 14: _t->newDesktopWindow((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 15: _t->userDesktopCursor((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const DesktopInput(*)>(_a[2]))); break;
        case 16: _t->newTextMessage((*reinterpret_cast< const TextMessage(*)>(_a[1]))); break;
        case 17: _t->filetransferUpdate((*reinterpret_cast< const FileTransfer(*)>(_a[1]))); break;
        case 18: _t->mediaStreamUpdate((*reinterpret_cast< const MediaFileInfo(*)>(_a[1]))); break;
        case 19: _t->mediaPlaybackUpdate((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const MediaFileInfo(*)>(_a[2]))); break;
        case 20: _t->cmdSuccess((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 21: _t->cmdError((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 22: _t->preferencesModified(); break;
        case 23: _t->slotClientNewInstance((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 24: _t->slotClientNewInstance(); break;
        case 25: _t->slotClientConnect((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 26: _t->slotClientConnect(); break;
        case 27: _t->slotClientPreferences((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 28: _t->slotClientPreferences(); break;
        case 29: _t->slotClientExit((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 30: _t->slotClientExit(); break;
        case 31: _t->slotMeChangeNickname((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 32: _t->slotMeChangeNickname(); break;
        case 33: _t->slotMeChangeStatus((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 34: _t->slotMeChangeStatus(); break;
        case 35: _t->slotMeEnablePushToTalk((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 36: _t->slotMeEnablePushToTalk(); break;
        case 37: _t->slotMeEnableVoiceActivation((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 38: _t->slotMeEnableVoiceActivation(); break;
        case 39: _t->slotMeEnableVideoTransmission((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 40: _t->slotMeEnableVideoTransmission(); break;
        case 41: _t->slotMeEnableDesktopSharing((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 42: _t->slotMeEnableDesktopSharing(); break;
        case 43: _t->slotUsersViewUserInformation((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 44: _t->slotUsersViewUserInformation(); break;
        case 45: _t->slotUsersMessages((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 46: _t->slotUsersMessages(); break;
        case 47: _t->slotUsersMuteVoice((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 48: _t->slotUsersMuteVoice(); break;
        case 49: _t->slotUsersMuteMediaFile((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 50: _t->slotUsersMuteMediaFile(); break;
        case 51: _t->slotUsersVolume((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 52: _t->slotUsersVolume(); break;
        case 53: _t->slotUsersOp((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 54: _t->slotUsersOp(); break;
        case 55: _t->slotUsersKickFromChannel((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 56: _t->slotUsersKickFromChannel(); break;
        case 57: _t->slotUsersKickBanFromChannel((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 58: _t->slotUsersKickBanFromChannel(); break;
        case 59: _t->slotUsersKickFromServer((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 60: _t->slotUsersKickFromServer(); break;
        case 61: _t->slotUsersKickBanFromServer((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 62: _t->slotUsersKickBanFromServer(); break;
        case 63: _t->slotUsersSubscriptionsUserMsg((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 64: _t->slotUsersSubscriptionsUserMsg(); break;
        case 65: _t->slotUsersSubscriptionsChannelMsg((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 66: _t->slotUsersSubscriptionsChannelMsg(); break;
        case 67: _t->slotUsersSubscriptionsBCastMsg((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 68: _t->slotUsersSubscriptionsBCastMsg(); break;
        case 69: _t->slotUsersSubscriptionsVoice((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 70: _t->slotUsersSubscriptionsVoice(); break;
        case 71: _t->slotUsersSubscriptionsVideo((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 72: _t->slotUsersSubscriptionsVideo(); break;
        case 73: _t->slotUsersSubscriptionsDesktop((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 74: _t->slotUsersSubscriptionsDesktop(); break;
        case 75: _t->slotUsersSubscriptionsDesktopInput((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 76: _t->slotUsersSubscriptionsDesktopInput(); break;
        case 77: _t->slotUsersSubscriptionsMediaFile((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 78: _t->slotUsersSubscriptionsMediaFile(); break;
        case 79: _t->slotUsersSubscriptionsInterceptUserMsg((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 80: _t->slotUsersSubscriptionsInterceptUserMsg(); break;
        case 81: _t->slotUsersSubscriptionsInterceptChannelMsg((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 82: _t->slotUsersSubscriptionsInterceptChannelMsg(); break;
        case 83: _t->slotUsersSubscriptionsInterceptVoice((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 84: _t->slotUsersSubscriptionsInterceptVoice(); break;
        case 85: _t->slotUsersSubscriptionsInterceptVideo((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 86: _t->slotUsersSubscriptionsInterceptVideo(); break;
        case 87: _t->slotUsersSubscriptionsInterceptDesktop((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 88: _t->slotUsersSubscriptionsInterceptDesktop(); break;
        case 89: _t->slotUsersSubscriptionsInterceptMediaFile((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 90: _t->slotUsersSubscriptionsInterceptMediaFile(); break;
        case 91: _t->slotUsersAdvancedIncVolumeVoice(); break;
        case 92: _t->slotUsersAdvancedDecVolumeVoice(); break;
        case 93: _t->slotUsersAdvancedIncVolumeMediaFile(); break;
        case 94: _t->slotUsersAdvancedDecVolumeMediaFile(); break;
        case 95: _t->slotUsersAdvancedStoreForMove(); break;
        case 96: _t->slotUsersAdvancedMoveUsers(); break;
        case 97: _t->slotUsersAdvancedVoiceAllowed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 98: _t->slotUsersAdvancedVoiceAllowed(); break;
        case 99: _t->slotUsersAdvancedVideoAllowed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 100: _t->slotUsersAdvancedVideoAllowed(); break;
        case 101: _t->slotUsersAdvancedDesktopAllowed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 102: _t->slotUsersAdvancedDesktopAllowed(); break;
        case 103: _t->slotUsersAdvancedMediaFileAllowed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 104: _t->slotUsersAdvancedMediaFileAllowed(); break;
        case 105: _t->slotUsersMuteVoiceAll((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 106: _t->slotUsersMuteVoiceAll(); break;
        case 107: _t->slotUsersStoreAudioToDisk((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 108: _t->slotUsersStoreAudioToDisk(); break;
        case 109: _t->slotChannelsCreateChannel((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 110: _t->slotChannelsCreateChannel(); break;
        case 111: _t->slotChannelsUpdateChannel((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 112: _t->slotChannelsUpdateChannel(); break;
        case 113: _t->slotChannelsDeleteChannel((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 114: _t->slotChannelsDeleteChannel(); break;
        case 115: _t->slotChannelsJoinChannel((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 116: _t->slotChannelsJoinChannel(); break;
        case 117: _t->slotChannelsViewChannelInfo((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 118: _t->slotChannelsViewChannelInfo(); break;
        case 119: _t->slotChannelsListBans((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 120: _t->slotChannelsListBans(); break;
        case 121: _t->slotChannelsStreamMediaFile((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 122: _t->slotChannelsStreamMediaFile(); break;
        case 123: _t->slotChannelsUploadFile((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 124: _t->slotChannelsUploadFile(); break;
        case 125: _t->slotChannelsDownloadFile((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 126: _t->slotChannelsDownloadFile(); break;
        case 127: _t->slotChannelsDeleteFile((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 128: _t->slotChannelsDeleteFile(); break;
        case 129: _t->slotServerUserAccounts((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 130: _t->slotServerUserAccounts(); break;
        case 131: _t->slotServerBannedUsers((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 132: _t->slotServerBannedUsers(); break;
        case 133: _t->slotServerOnlineUsers((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 134: _t->slotServerOnlineUsers(); break;
        case 135: _t->slotServerBroadcastMessage((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 136: _t->slotServerBroadcastMessage(); break;
        case 137: _t->slotServerServerProperties((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 138: _t->slotServerServerProperties(); break;
        case 139: _t->slotServerSaveConfiguration((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 140: _t->slotServerSaveConfiguration(); break;
        case 141: _t->slotServerServerStatistics((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 142: _t->slotServerServerStatistics(); break;
        case 143: _t->slotHelpManual((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 144: _t->slotHelpManual(); break;
        case 145: _t->slotHelpResetPreferences((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 146: _t->slotHelpResetPreferences(); break;
        case 147: _t->slotHelpVisitBearWare((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 148: _t->slotHelpVisitBearWare(); break;
        case 149: _t->slotHelpVisitChangeLog((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 150: _t->slotHelpVisitChangeLog(); break;
        case 151: _t->slotHelpAbout((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 152: _t->slotHelpAbout(); break;
        case 153: _t->slotConnectToLatest(); break;
        case 154: _t->slotUsersViewUserInformation((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 155: _t->slotUsersMessages((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 156: _t->slotUsersMuteVoice((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 157: _t->slotUsersMuteMediaFile((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 158: _t->slotUsersVolume((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 159: _t->slotUsersOp((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 160: _t->slotUsersKick((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 161: _t->slotUsersKickBan((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 162: _t->slotTreeSelectionChanged(); break;
        case 163: _t->slotTreeContextMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 164: _t->slotUpdateUI(); break;
        case 165: _t->slotUpdateVideoTabUI(); break;
        case 166: _t->slotUpdateDesktopTabUI(); break;
        case 167: _t->slotUploadFiles((*reinterpret_cast< const QStringList(*)>(_a[1]))); break;
        case 168: _t->slotSendChannelMessage(); break;
        case 169: _t->slotUserDoubleClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 170: _t->slotChannelDoubleClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 171: _t->slotNewTextMessage((*reinterpret_cast< const TextMessage(*)>(_a[1]))); break;
        case 172: _t->slotNewMyselfTextMessage((*reinterpret_cast< const TextMessage(*)>(_a[1]))); break;
        case 173: _t->slotTextMessageClosed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 174: _t->slotTransmitUsersChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QMap<int,StreamTypes>(*)>(_a[2]))); break;
        case 175: _t->slotChannelUpdate((*reinterpret_cast< const Channel(*)>(_a[1]))); break;
        case 176: _t->slotInitVideo(); break;
        case 177: _t->slotAddUserVideo(); break;
        case 178: _t->slotRemoveUserVideo(); break;
        case 179: _t->slotRemoveUserVideo((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 180: _t->slotDetachUserVideo((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 181: _t->slotDetachUserVideo(); break;
        case 182: _t->slotNewUserVideoDlg((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2]))); break;
        case 183: _t->slotUserVideoDlgClosing((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 184: _t->slotAddUserDesktop(); break;
        case 185: _t->slotRemoveUserDesktop(); break;
        case 186: _t->slotRemoveUserDesktop((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 187: _t->slotAccessUserDesktop((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 188: _t->slotDetachUserDesktop(); break;
        case 189: _t->slotDetachUserDesktop((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2]))); break;
        case 190: _t->slotUserDesktopDlgClosing((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 191: _t->slotUserJoin((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const User(*)>(_a[2]))); break;
        case 192: _t->slotUserLeft((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const User(*)>(_a[2]))); break;
        case 193: _t->slotUserUpdate((*reinterpret_cast< const User(*)>(_a[1]))); break;
        case 194: _t->slotEnableQuestionMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 195: _t->slotUpdateVideoCount((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 196: _t->slotUpdateDesktopCount((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 197: _t->slotMasterVolumeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 198: _t->slotMicrophoneGainChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 199: _t->slotVoiceActivationLevelChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 200: _t->slotTrayIconChange((*reinterpret_cast< QSystemTrayIcon::ActivationReason(*)>(_a[1]))); break;
        case 201: _t->slotLoadTTFile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 202: _t->slotSoftwareUpdateReply((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 203: _t->slotBearWareAuthReply((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 204: _t->slotClosedOnlineUsersDlg((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 205: _t->slotClosedServerStatsDlg((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 206: _t->slotClosedUserAccountsDlg((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 207: _t->slotClosedBannedUsersDlg((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MainWindow::*)(const ServerProperties & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::serverUpdate)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const ServerStatistics & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::serverStatistics)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const Channel & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::newChannel)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const Channel & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::updateChannel)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const Channel & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::removeChannel)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const User & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::userLogin)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const User & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::userLogout)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const User & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::userUpdate)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(int , const User & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::userJoined)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(int , const User & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::userLeft)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const User & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::userStateChange)) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::updateMyself)) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::newVideoCaptureFrame)) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::newMediaVideoFrame)) {
                *result = 13;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::newDesktopWindow)) {
                *result = 14;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(int , const DesktopInput & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::userDesktopCursor)) {
                *result = 15;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const TextMessage & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::newTextMessage)) {
                *result = 16;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const FileTransfer & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::filetransferUpdate)) {
                *result = 17;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const MediaFileInfo & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::mediaStreamUpdate)) {
                *result = 18;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(int , const MediaFileInfo & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::mediaPlaybackUpdate)) {
                *result = 19;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::cmdSuccess)) {
                *result = 20;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::cmdError)) {
                *result = 21;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::preferencesModified)) {
                *result = 22;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 208)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 208;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 208)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 208;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::serverUpdate(const ServerProperties & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MainWindow::serverStatistics(const ServerStatistics & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void MainWindow::newChannel(const Channel & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void MainWindow::updateChannel(const Channel & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void MainWindow::removeChannel(const Channel & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void MainWindow::userLogin(const User & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void MainWindow::userLogout(const User & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void MainWindow::userUpdate(const User & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void MainWindow::userJoined(int _t1, const User & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void MainWindow::userLeft(int _t1, const User & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void MainWindow::userStateChange(const User & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void MainWindow::updateMyself()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void MainWindow::newVideoCaptureFrame(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void MainWindow::newMediaVideoFrame(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void MainWindow::newDesktopWindow(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void MainWindow::userDesktopCursor(int _t1, const DesktopInput & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}

// SIGNAL 16
void MainWindow::newTextMessage(const TextMessage & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 16, _a);
}

// SIGNAL 17
void MainWindow::filetransferUpdate(const FileTransfer & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 17, _a);
}

// SIGNAL 18
void MainWindow::mediaStreamUpdate(const MediaFileInfo & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 18, _a);
}

// SIGNAL 19
void MainWindow::mediaPlaybackUpdate(int _t1, const MediaFileInfo & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 19, _a);
}

// SIGNAL 20
void MainWindow::cmdSuccess(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 20, _a);
}

// SIGNAL 21
void MainWindow::cmdError(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 21, _a);
}

// SIGNAL 22
void MainWindow::preferencesModified()
{
    QMetaObject::activate(this, &staticMetaObject, 22, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
