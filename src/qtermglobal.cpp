//
// C++ Implementation: Global
//
// Description:
//
//
// Author: hooey <hephooey@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "qtermglobal.h"
#include "qtermconfig.h"
#include "qtermparam.h"
#include "qtermconvert.h"
#ifdef KWALLET_ENABLED
#include "wallet.h"
#endif // KWALLET_ENABLED
#include "qterm.h"
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTranslator>
#include <QtCore/QVariant>
#include <QtCore/QUrl>
#include <QtCore/QProcess>
#include <QtCore/QLibraryInfo>
#include <QtGui/QApplication>
#include <QtGui/QDesktopServices>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)
#ifndef MAX_PATH
#define MAX_PATH 128
#endif
#include <windows.h>
#include <shellapi.h>
#endif

namespace QTerm
{

Global * Global::m_Instance = 0;

Global * Global::instance()
{
    static QMutex mutex;
    if (!m_Instance) {
        mutex.lock();

        if (!m_Instance)
            m_Instance = new Global;

        mutex.unlock();
    }

    return m_Instance;
}

Global::Global()
        : m_fileCfg("./qterm.cfg"), m_addrCfg("./address.cfg"), m_pathLib("./"), m_pathPic("./"), m_pathCfg("./"), m_windowState(), m_status(INIT_OK), m_style(), m_fullScreen(false), m_language(Global::English), m_showToolBar()
{
    if (!iniWorkingDir(qApp->arguments()[0])) {
        m_status = INIT_ERROR;
        return;
    }
    m_config = new Config(m_fileCfg);
    m_address = new Config(m_addrCfg);
    m_converter = new Convert();
#ifdef KWALLET_ENABLED
    if (Wallet::isWalletAvailable()) {
        qDebug() << "KWallet service found";
        m_wallet = new Wallet(this);
    } else {
        m_wallet = NULL;
    }
#endif // KWALLET_ENABLED
    if (!iniSettings()) {
        m_status = INIT_ERROR;
        return;
    }
}

bool Global::isOK()
{
    if (m_status == INIT_OK) {
        return true;
    }
    return false;
}

Config * Global::fileCfg()
{
    return m_config;
}

Config * Global::addrCfg()
{
    return m_address;
}

const QString & Global::pathLib()
{
    return m_pathLib;
}

const QString & Global::pathPic()
{
    return m_pathPic;
}

const QString & Global::pathCfg()
{
    return m_pathCfg;
}

void Global::clearDir(const QString & path)
{
    QDir dir(path);
    if (dir.exists()) {
        const QFileInfoList list = dir.entryInfoList();
        foreach(QFileInfo fi, list) {
            if (fi.isFile())
                dir.remove(fi.fileName());
        }
    }
}

QStringList Global::loadNameList()
{
    QStringList listName;

    QString strTmp = m_address->getItemValue("bbs list", "num").toString();

    QString strSection;

    for (int i = 0; i < strTmp.toInt(); i++) {
        strSection.sprintf("bbs %d", i);
        listName.append(m_address->getItemValue(strSection, "name").toString());
    }

    return listName;
}

bool Global::loadAddress(int n, Param& param)
{
    QString strTmp, strSection;
    strTmp = m_address->getItemValue("bbs list", "num").toString();
    if ((n < 0 && strTmp.toInt() <= 0) || n < -1)
        strSection = "default";
    else {
        n = n < 0 ? 0 : n;
        strSection.sprintf("bbs %d", n);
    }

    // check if larger than existence
    strTmp = m_address->getItemValue("bbs list", "num").toString();
    if (n >= strTmp.toInt())
        return false;
    param.m_strName = m_address->getItemValue(strSection, "name").toString();
    param.m_strAddr = m_address->getItemValue(strSection, "addr").toString();
    strTmp = m_address->getItemValue(strSection, "port").toString();
    param.m_uPort = strTmp.toUShort();
    strTmp = m_address->getItemValue(strSection, "hosttype").toString();
    param.m_nHostType = strTmp.toInt();
    strTmp = m_address->getItemValue(strSection, "autologin").toString();
    param.m_bAutoLogin = (strTmp != "0");
    param.m_strPreLogin = m_address->getItemValue(strSection, "prelogin").toString();
    param.m_strUser = m_address->getItemValue(strSection, "user").toString();
#ifdef KWALLET_ENABLED
    if (m_wallet != NULL) {
        m_wallet->open();
        param.m_strPasswd = m_wallet->readPassword(param.m_strName, param.m_strUser);
    } else
#endif // KWALLET_ENABLED
        param.m_strPasswd = m_address->getItemValue(strSection, "password").toString();
    param.m_strPostLogin = m_address->getItemValue(strSection, "postlogin").toString();

    strTmp = m_address->getItemValue(strSection, "bbscode").toString();
    param.m_BBSCode = strTmp;
    strTmp = m_address->getItemValue(strSection, "displaycode").toString();
    param.m_nDispCode = strTmp.toInt();
    strTmp = m_address->getItemValue(strSection, "autofont").toString();
    param.m_bAutoFont = (strTmp != "0");
    strTmp = m_address->getItemValue(strSection, "alwayshighlight").toString();
    param.m_bAlwaysHighlight = (strTmp != "0");
    strTmp = m_address->getItemValue(strSection, "ansicolor").toString();
    param.m_bAnsiColor = (strTmp != "0");
    param.m_strASCIIFontName = m_address->getItemValue(strSection, "asciifont").toString();
    param.m_strGeneralFontName = m_address->getItemValue(strSection, "generalfont").toString();
    strTmp = m_address->getItemValue(strSection, "fontsize").toString();
    param.m_nFontSize = strTmp.toInt();
    param.m_strSchemeFile = m_address->getItemValue(strSection, "schemefile").toString();
    param.m_strKeyboardProfile = m_address->getItemValue(strSection, "keyboardprofile").toString();

    param.m_strTerm = m_address->getItemValue(strSection, "termtype").toString();
    strTmp =  m_address->getItemValue(strSection, "column").toString();
    param.m_nCol = strTmp.toInt();
    strTmp =  m_address->getItemValue(strSection, "row").toString();
    param.m_nRow = strTmp.toInt();
    strTmp =  m_address->getItemValue(strSection, "scroll").toString();
    param.m_nScrollLines = strTmp.toInt();
    strTmp =  m_address->getItemValue(strSection, "cursor").toString();
    param.m_nCursorType = strTmp.toInt();
    param.m_strEscape = m_address->getItemValue(strSection, "escape").toString();

    strTmp =  m_address->getItemValue(strSection, "proxytype").toString();
    param.m_nProxyType = strTmp.toInt();
    strTmp = m_address->getItemValue(strSection, "proxyauth").toString();
    param.m_bAuth = (strTmp != "0");
    param.m_strProxyHost = m_address->getItemValue(strSection, "proxyaddr").toString();
    strTmp = m_address->getItemValue(strSection, "proxyport").toString();
    param.m_uProxyPort = strTmp.toInt();
    param.m_strProxyUser = m_address->getItemValue(strSection, "proxyuser").toString();
    param.m_strProxyPasswd = m_address->getItemValue(strSection, "proxypassword").toString();
    strTmp = m_address->getItemValue(strSection, "protocol").toString();
    param.m_nProtocolType = strTmp.toInt();

    strTmp = m_address->getItemValue(strSection, "maxidle").toString();
    param.m_nMaxIdle = strTmp.toInt();
    param.m_strReplyKey = m_address->getItemValue(strSection, "replykey").toString();
    if (param.m_strReplyKey.isNull())
        qDebug("loading null\n");

    param.m_strAntiString = m_address->getItemValue(strSection, "antiidlestring").toString();
    param.m_strAutoReply = m_address->getItemValue(strSection, "autoreply").toString();
    strTmp = m_address->getItemValue(strSection, "bautoreply").toString();
    param.m_bAutoReply = (strTmp != "0");

    strTmp = m_address->getItemValue(strSection, "reconnect").toString();
    param.m_bReconnect = (strTmp != "0");
    strTmp = m_address->getItemValue(strSection, "interval").toString();
    param.m_nReconnectInterval = strTmp.toInt();
    strTmp = m_address->getItemValue(strSection, "retrytimes").toString();
    param.m_nRetry = strTmp.toInt();

    strTmp = m_address->getItemValue(strSection, "loadscript").toString();
    param.m_bLoadScript = (strTmp != "0");
    param.m_strScriptFile = m_address->getItemValue(strSection, "scriptfile").toString();
    if (param.m_strScriptFile.isEmpty()) {
        param.m_bLoadScript = false;
    }

    strTmp = m_address->getItemValue(strSection, "menutype").toString();
    param.m_nMenuType = strTmp.toInt();
    param.m_clrMenu.setNamedColor(m_address->getItemValue(strSection, "menucolor").toString());

    return true;
}

void Global::saveAddress(int n, const Param& param)
{
    QString strTmp, strSection;
    if (n < 0)
        strSection = "default";
    else
        strSection.sprintf("bbs %d", n);

    m_address->setItemValue(strSection, "name", param.m_strName);
    m_address->setItemValue(strSection, "addr", param.m_strAddr);
    strTmp.setNum(param.m_uPort);
    m_address->setItemValue(strSection, "port", strTmp);
    strTmp.setNum(param.m_nHostType);
    m_address->setItemValue(strSection, "hosttype", strTmp);
    m_address->setItemValue(strSection, "autologin", param.m_bAutoLogin ? "1" : "0");
    m_address->setItemValue(strSection, "prelogin", param.m_strPreLogin);
    m_address->setItemValue(strSection, "user", param.m_strUser);

#ifdef KWALLET_ENABLED
    if (m_wallet != NULL) {
        m_wallet->open();
        m_wallet->writePassword(param.m_strName, param.m_strUser, param.m_strPasswd);
    } else
#endif // KWALLET_ENABLED
        m_address->setItemValue(strSection, "password", param.m_strPasswd);
    m_address->setItemValue(strSection, "postlogin", param.m_strPostLogin);

    strTmp=param.m_BBSCode;
    m_address->setItemValue(strSection, "bbscode", strTmp);
    strTmp.setNum(param.m_nDispCode);
    m_address->setItemValue(strSection, "displaycode", strTmp);
    m_address->setItemValue(strSection, "autofont", param.m_bAutoFont ? "1" : "0");
    m_address->setItemValue(strSection, "alwayshighlight", param.m_bAlwaysHighlight ? "1" : "0");
    m_address->setItemValue(strSection, "ansicolor", param.m_bAnsiColor ? "1" : "0");
    m_address->setItemValue(strSection, "asciifont", param.m_strASCIIFontName);
    m_address->setItemValue(strSection, "generalfont", param.m_strGeneralFontName);
    strTmp.setNum(param.m_nFontSize);
    m_address->setItemValue(strSection, "fontsize", strTmp);
    m_address->setItemValue(strSection, "schemefile", param.m_strSchemeFile);
    m_address->setItemValue(strSection, "keyboardprofile", param.m_strKeyboardProfile);

    m_address->setItemValue(strSection, "termtype", param.m_strTerm);
    strTmp.setNum(param.m_nCol);
    m_address->setItemValue(strSection, "column", strTmp);
    strTmp.setNum(param.m_nRow);
    m_address->setItemValue(strSection, "row", strTmp);
    strTmp.setNum(param.m_nScrollLines);
    m_address->setItemValue(strSection, "scroll", strTmp);
    strTmp.setNum(param.m_nCursorType);
    m_address->setItemValue(strSection, "cursor", strTmp);
    m_address->setItemValue(strSection, "escape", param.m_strEscape);

    strTmp.setNum(param.m_nProxyType);
    m_address->setItemValue(strSection, "proxytype", strTmp);
    m_address->setItemValue(strSection, "proxyauth", param.m_bAuth ? "1" : "0");
    m_address->setItemValue(strSection, "proxyaddr", param.m_strProxyHost);
    strTmp.setNum(param.m_uProxyPort);
    m_address->setItemValue(strSection, "proxyport", strTmp);
    m_address->setItemValue(strSection, "proxyuser", param.m_strProxyUser);
    m_address->setItemValue(strSection, "proxypassword", param.m_strProxyPasswd);
    strTmp.setNum(param.m_nProtocolType);
    m_address->setItemValue(strSection, "protocol", strTmp);

    strTmp.setNum(param.m_nMaxIdle);
    m_address->setItemValue(strSection, "maxidle", strTmp);
    m_address->setItemValue(strSection, "replykey", param.m_strReplyKey);
    m_address->setItemValue(strSection, "antiidlestring", param.m_strAntiString);
    m_address->setItemValue(strSection, "bautoreply", param.m_bAutoReply ? "1" : "0");
    m_address->setItemValue(strSection, "autoreply", param.m_strAutoReply);
    m_address->setItemValue(strSection, "reconnect", param.m_bReconnect ? "1" : "0");
    strTmp.setNum(param.m_nReconnectInterval);
    m_address->setItemValue(strSection, "interval", strTmp);
    strTmp.setNum(param.m_nRetry);
    m_address->setItemValue(strSection, "retrytimes", strTmp);

    m_address->setItemValue(strSection, "loadscript", param.m_bLoadScript ? "1" : "0");
    m_address->setItemValue(strSection, "scriptfile", param.m_strScriptFile);

    strTmp.setNum(param.m_nMenuType);
    m_address->setItemValue(strSection, "menutype", strTmp);
    m_address->setItemValue(strSection, "menucolor", param.m_clrMenu.name());
    m_address->save();

}

void Global::removeAddress(int n)
{
    if (n < 0)
        return;
    QString strSection = QString("bbs %1").arg(n);
#ifdef KWALLET_ENABLED
    // check if larger than existence
    QString strTmp = m_address->getItemValue("bbs list", "num").toString();
    if (n >= strTmp.toInt())
        return;
    QString site = m_address->getItemValue(strSection, "name").toString();
    QString username = m_address->getItemValue(strSection, "user").toString();
    if (m_wallet != NULL) {
        m_wallet->open();
        m_wallet->removePassword(site, username);
    }
#endif // KWALLET_ENABLED
    m_address->deleteSection(strSection);
}

void Global::loadPrefence()
{
    QString strTmp;
    strTmp = m_config->getItemValue("preference", "xim").toString();
    m_pref.XIM = (Global::Conversion)strTmp.toInt();
    strTmp = m_config->getItemValue("preference", "wordwrap").toString();
    m_pref.nWordWrap = strTmp.toInt();
    strTmp = m_config->getItemValue("preference", "wheel").toString();
    m_pref.bWheel = (strTmp != "0");
    strTmp = m_config->getItemValue("preference", "url").toString();
    m_pref.bUrl = (strTmp != "0");
    strTmp = m_config->getItemValue("preference", "blinktab").toString();
    m_pref.bBlinkTab = (strTmp != "0");
    strTmp = m_config->getItemValue("preference", "warn").toString();
    m_pref.bWarn = (strTmp != "0");
    strTmp = m_config->getItemValue("preference", "beep").toString();
    m_pref.nBeep = strTmp.toInt();
    m_pref.strWave = m_config->getItemValue("preference", "wavefile").toString();
    strTmp = m_config->getItemValue("preference", "http").toString();
    m_pref.strHttp = strTmp;
    strTmp = m_config->getItemValue("preference", "antialias").toString();
    m_pref.bAA = (strTmp != "0");
    strTmp = m_config->getItemValue("preference", "tray").toString();
    m_pref.bTray = (strTmp != "0");
    strTmp = m_config->getItemValue("preference", "externalplayer").toString();
    m_pref.strPlayer = strTmp;

    strTmp = m_config->getItemValue("preference", "clearpool").toString();
    m_pref.bClearPool = (strTmp != "0");
    strTmp = m_config->getItemValue("preference", "pool").toString();
    m_pref.strPoolPath = strTmp.isEmpty() ? Global::instance()->pathCfg() + "pool/" : strTmp;
    if (m_pref.strPoolPath.right(1) != "/")
        m_pref.strPoolPath.append('/');
    strTmp = m_config->getItemValue("preference", "zmodem").toString();
    m_pref.strZmPath = strTmp.isEmpty() ? Global::instance()->pathCfg() + "zmodem/" : strTmp;
    if (m_pref.strZmPath.right(1) != "/")
        m_pref.strZmPath.append('/');
    strTmp = m_config->getItemValue("preference", "image").toString();
    m_pref.strImageViewer = strTmp;
}

QString Global::getOpenFileName(const QString & filter, QWidget * widget)
{
    QString path = m_config->getItemValue("global", "openfiledialog").toString();

    QString strOpen = QFileDialog::getOpenFileName(widget, "choose a file", path, filter);

    if (!strOpen.isEmpty()) {
        // save the path
        QFileInfo fi(strOpen);
        m_config->setItemValue("global", "openfiledialog", fi.absolutePath());
        m_config->save();
    }

    return strOpen;
}

// warning for overwrite getSaveFileName
QString Global::getSaveFileName(const QString& filename, QWidget* widget)
{
    // get the previous dir
    QString path = m_config->getItemValue("global", "savefiledialog").toString();

    QString strSave = QFileDialog::getSaveFileName(widget, tr("Choose a file to save under"), path + "/" + filename, "*");

    QFileInfo fi(strSave);

    while (fi.exists()) {
        int yn = QMessageBox::warning(widget, "QTerm",
                                      tr("File exists. Overwrite?"), tr("Yes"), tr("No"));
        if (yn == 0)
            break;
        strSave = QFileDialog::getSaveFileName(widget, tr("Choose a file to save under"), path + "/" + filename, "*");
        if (strSave.isEmpty())
            break;
    }

    if (!strSave.isEmpty()) {
        // save the path
        m_config->setItemValue("global", "savefiledialog", fi.absolutePath());
        m_config->save();
    }

    return strSave;
}

#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)
bool Global::iniWorkingDir(QString param)
{
    char ExeNamePath[MAX_PATH], _fileCfg[MAX_PATH], _addrCfg[MAX_PATH];
    size_t LastSlash = 0;

    if (0 == GetModuleFileNameA(NULL, ExeNamePath, MAX_PATH)) {
        const char *DefaultModuleName = "./qterm.exe";
        strcpy(ExeNamePath, DefaultModuleName);
    }
    for (size_t i = 0, ModuleNameLen = strlen(ExeNamePath) ;
            i < ModuleNameLen ; ++i) {
        if (ExeNamePath[i] == '\\') {
            ExeNamePath[i] = '/';
            LastSlash = i;
        }
    }
    ExeNamePath[LastSlash+1] = '\0';
    m_pathLib = QString::fromLocal8Bit(ExeNamePath);
    m_pathPic = QString::fromLocal8Bit(ExeNamePath);
    m_pathCfg = QString::fromLocal8Bit(ExeNamePath);
    strcpy(_fileCfg, ExeNamePath);
    strcat(_fileCfg, "qterm.cfg");
    m_fileCfg = QString::fromLocal8Bit(_fileCfg);
    strcpy(_addrCfg, ExeNamePath);
    strcat(_addrCfg, "address.cfg");
    m_addrCfg = QString::fromLocal8Bit(_addrCfg);

    QString pathScheme = m_pathCfg + "scheme";
    if (!isPathExist(pathScheme))
        return false;
    return true;
}
#else
bool Global::iniWorkingDir(QString param)
{
    QDir dir;
    QFileInfo fi;
#ifdef Q_OS_MACX
    // $HOME/Library/QTerm/
    QString pathHome = QDir::homePath();
    m_pathCfg = pathHome + "/Library/QTerm/";
    if (!isPathExist(m_pathCfg))
        return false;

    // get executive file path
    fi.setFile(param);
    m_pathLib = fi.path() + '/';
#else
    QString prefix = QCoreApplication::applicationDirPath();

    QFileInfo conf(prefix+"/qterm.cfg");
    if (conf.exists()) {
        QString path= QCoreApplication::applicationDirPath()+"/";
        m_pathLib = path;
        m_pathPic = path;
        m_pathCfg = path;
        return true;
    }

    prefix.chop(3); // "bin"
    m_pathCfg = QDir::homePath() + "/.qterm/";
    if (!isPathExist(m_pathCfg))
        return false;

    // pathLib --- where datedir "pic", "cursor", "po"
    if (param.indexOf('/') == -1)
        m_pathLib = prefix + "share/qterm/";
    else {
        QFileInfo fi(param);
        if (fi.isSymLink())
            param = fi.readLink();
        // get the pathname
        param.truncate(param.lastIndexOf('/'));
        QString oldPath = QDir::currentPath();
        QDir::setCurrent(param);
        dir.setPath(QCoreApplication::applicationDirPath());
        if (dir == QDir::current())
            m_pathLib = prefix + "share/qterm/";
        else
            m_pathLib = QDir::currentPath();
        QDir::setCurrent(oldPath);
        m_pathLib += '/';
    }
#endif

    QString pathScheme = m_pathCfg + "scheme";
    if (!isPathExist(pathScheme))
        return false;


    // picPath --- $HOME/.qterm/pic prefered
    m_pathPic = m_pathCfg + "pic";
    dir.setPath(m_pathPic);
    if (!dir.exists())
        m_pathPic = m_pathLib;
    else
        m_pathPic = m_pathCfg;

    // copy configuration files
    m_fileCfg = m_pathCfg + "qterm.cfg";
    if (!createLocalFile(m_fileCfg, m_pathLib + "qterm.cfg"))
        return false;
    m_addrCfg = m_pathCfg + "address.cfg";
    if (!createLocalFile(m_addrCfg, m_pathLib + "address.cfg"))
        return false;

    return true;
}
#endif

bool Global::iniSettings()
{
    //install the translator
    QString lang = m_config->getItemValue("global", "language").toString();
    if (lang == "eng")
        m_language = Global::English;
    else if (lang == "chs")
        m_language = Global::SimpilifiedChinese;
    else if (lang == "cht")
        m_language = Global::TraditionalChinese;
    else {
        qDebug("Language setting is not correct");
        m_language = Global::English;
    }
    if (lang != "eng" && !lang.isEmpty()) {
        QString qt_qm;
        if (lang == "chs")
            qt_qm = QLibraryInfo::location(QLibraryInfo::TranslationsPath)+"/qt_zh_CN.qm";
        else
            qt_qm = QLibraryInfo::location(QLibraryInfo::TranslationsPath)+"/qt_zh_TW.qm";

        static QTranslator * translator = new QTranslator(0);
        translator->load(qt_qm);
        qApp->installTranslator(translator);

        // look in $HOME/.qterm/po/ first
        QString qterm_qm = QDir::homePath() + "/.qterm/po/qterm_" + lang + ".qm";
        if (!QFile::exists(qterm_qm))
            qterm_qm = m_pathLib + "po/qterm_" + lang + ".qm";
        translator = new QTranslator(0);
        translator->load(qterm_qm);
        qApp->installTranslator(translator);
    }
    //set font
    QString family = m_config->getItemValue("global", "font").toString();
    QString pointsize = m_config->getItemValue("global", "pointsize").toString();
    if (!family.isEmpty()) {
        QFont font(family);
        if (pointsize.toInt() > 0)
            font.setPointSize(pointsize.toInt());
        QString bAA = m_config->getItemValue("global", "antialias").toString();
        if (bAA != "0")
            font.setStyleStrategy(QFont::PreferAntialias);
        qApp->setFont(font);
    }

    // zmodem and pool directory
    QString pathZmodem = m_config->getItemValue("preference", "zmodem").toString();
    if (pathZmodem.isEmpty())
        pathZmodem = m_pathCfg + "zmodem";
    if (!isPathExist(pathZmodem))
        return false;

    QString pathPool = m_config->getItemValue("preference", "pool").toString();

    if (pathPool.isEmpty())
        pathPool = m_pathCfg + "pool/";

    if (pathPool.right(1) != "/")
        pathPool.append('/');

    QString pathCache = pathPool + "shadow-cache/";

    if ((!isPathExist(pathPool)) || (!isPathExist(pathCache)))
        return false;

    return true;
}


bool Global::isPathExist(const QString& path)
{
    QDir dir(path);
    if (! dir.exists()) {
        if (!dir.mkdir(path)) {
            qDebug("Failed to create directory %s", (const char*)path.toLocal8Bit());
            return false;
        }
    }
    return true;
}

bool Global::createLocalFile(const QString& dst, const QString& src)
{
    QDir dir;
    if (dir.exists(dst))
        return true;

    if (!dir.exists(src)) {
        qDebug("QTerm failed to find %s.\n"
               "Please copy the %s from the source tarball to %s yourself.",
               (const char*)src.toLocal8Bit(), (const char*)src.toLocal8Bit(),
               (const char*) dst.toLocal8Bit());
        return false;
    }

    if (!QFile::copy(src, dst)) {
        qDebug("QTerm failed to copy %s to %s.\n"
               "Please copy the %s from the source tarball to %s yourself.",
               (const char*)src.toLocal8Bit(), (const char *)dst.toLocal8Bit(),
               (const char*)src.toLocal8Bit(), (const char *)dst.toLocal8Bit());
        return false;
    }
    return true;
}
bool Global::isBossColor() const
{
    return m_bossColor;
}

const QString & Global::escapeString() const
{
    return m_escape;
}

Global::Conversion Global::clipConversion() const
{
    return m_clipConversion;
}

Global::Language Global::language() const
{
    return m_language;
}

Global::Position Global::scrollPosition() const
{
    return m_scrollPos;
}

bool Global::isFullScreen() const
{
    return m_fullScreen;
}

bool Global::showSwitchBar() const
{
    return m_switchBar;
}

void Global::setClipConversion(Global::Conversion conversionId)
{
    m_clipConversion = conversionId;
}

void Global::setEscapeString(const QString & escapeString)
{
    m_escape = escapeString;
}

void Global::setScrollPosition(Global::Position position)
{
    m_scrollPos = position;
}

void Global::setStatusBar(bool isShow)
{
    m_statusBar = isShow;
}

void Global::setBossColor(bool isBossColor)
{
    m_bossColor = isBossColor;
}

void Global::setFullScreen(bool isFullscreen)
{
    m_fullScreen = isFullscreen;
}

void Global::setSwitchBar(bool isShow)
{
    m_switchBar = isShow;
}

void Global::setLanguage(Global::Language language)
{
    m_language = language;
}

const QString & Global::style() const
{
    return m_style;
}

void Global::setStyle(const QString & style)
{
    m_style = style;
}

void Global::loadConfig()
{
    QString strTmp;
    strTmp = m_config->getItemValue("global", "fullscreen").toString();

    if (strTmp == "1") {
        setFullScreen(true);
    } else {
        setFullScreen(false);
    }

    setStyle(m_config->getItemValue("global", "theme").toString());

    setEscapeString("");

    strTmp = m_config->getItemValue("global", "clipcodec").toString();
    setClipConversion((Global::Conversion)strTmp.toInt());

    strTmp = m_config->getItemValue("global", "vscrollpos").toString();
    if (strTmp == "0") {
        setScrollPosition(Global::Hide);
    } else if (strTmp == "1") {
        setScrollPosition(Global::Left);
    } else {
        setScrollPosition(Global::Right);
    }

    strTmp = m_config->getItemValue("global", "statusbar").toString();
    setStatusBar((strTmp != "0"));

    strTmp = m_config->getItemValue("global", "switchbar").toString();
    setSwitchBar((strTmp != "0"));

    setBossColor(false);

    loadPrefence();

}

bool Global::showToolBar(const QString & toolbar)
{
    if (m_showToolBar.contains(toolbar)) {
        return m_showToolBar.value(toolbar);
    } else {
        if (m_config->hasItem("ToolBars", toolbar+"Shown")) {
            bool isShown = m_config->getItemValue("ToolBars", toolbar+"Shown").toBool();
            m_showToolBar.insert(toolbar, isShown);
            return isShown;
        } else {
            // Show toolbar by default
            m_showToolBar.insert(toolbar, true);
            return true;
        }
    }
}

void Global::setShowToolBar(const QString & toolbar, bool isShown)
{
    m_showToolBar.insert(toolbar, isShown);
}

void Global::saveShowToolBar()
{
    QMapIterator<QString, bool> i(m_showToolBar);
    while (i.hasNext()) {
        i.next();
        m_config->setItemValue("ToolBars",i.key()+"Shown", i.value());
    }
}

void Global::saveConfig()
{

    QString strTmp;
    //save font
    m_config->setItemValue("global", "font", qApp->font().family());
    strTmp.setNum(QFontInfo(qApp->font()).pointSize());
    m_config->setItemValue("global", "pointsize", strTmp);

    if (isFullScreen())
        m_config->setItemValue("global", "fullscreen", "1");
    else
        m_config->setItemValue("global", "fullscreen", "0");

    // cstrTmp.setNum(theme);
    m_config->setItemValue("global", "theme", style());


    // Should we convert the numbers to strings like "GBK" and "Big5";
    strTmp.setNum(clipConversion());
    m_config->setItemValue("global", "clipcodec", strTmp);

    strTmp.setNum(scrollPosition());
    m_config->setItemValue("global", "vscrollpos", strTmp);

    m_config->setItemValue("global", "switchbar", showSwitchBar() ? "1" : "0");
    saveShowToolBar();
    m_config->save();

}

QByteArray Global::loadGeometry()
{
    return m_config->getItemValue("global","geometry").toByteArray();
}

QByteArray Global::loadState()
{
    return m_config->getItemValue("global","state").toByteArray();
}

void Global::saveGeometry(const QByteArray geometry)
{
    m_config->setItemValue("global", "geometry", geometry);
}

void Global::saveState(const QByteArray state)
{
    m_config->setItemValue("global", "state", state);
}

void Global::saveSession(const QList<QVariant>& sites)
{
    m_config->setItemValue("global", "sites", sites);
}

QList<QVariant> Global::loadSession()
{
    return m_config->getItemValue("global", "sites").toList();
}

void Global::cleanup()
{
#ifdef KWALLET_ENABLED
    if (m_wallet != NULL)
        m_wallet->close();
#endif // KWALLET_ENABLED
    if (m_pref.bClearPool) {
        clearDir(m_pref.strZmPath);
        clearDir(m_pref.strPoolPath);
        clearDir(m_pref.strPoolPath + "shadow-cache/");
    }
}

void Global::openUrl(const QString & urlStr)
{
    QString command = m_pref.strHttp;
    if (command.isEmpty()) {
        QUrl url(urlStr,QUrl::TolerantMode);
        if (!QDesktopServices::openUrl(url)) {
            qDebug("Failed to open the url with QDesktopServices");
        }
        return;
    }

    if(command.indexOf("%L")==-1) // no replace
        //QApplication::clipboard()->setText(strUrl);
        command += " \"" + urlStr +"\"";
    else
        command.replace("%L",  "\"" + urlStr + "\"");
        //cstrCmd.replace("%L",  strUrl.toLocal8Bit());

#if !defined(_OS_WIN32_) && !defined(Q_OS_WIN32)
    command += " &";
    system(command.toUtf8().data());
#else
    QProcess::startDetached(command);
#endif

}

QString Global::convert(const QString & source, Global::Conversion flag)
{
    switch (flag) {
    case No_Conversion:
        return source;
    case Simplified_To_Traditional:
        return m_converter->S2T(source);
    case Traditional_To_Simplified:
        return m_converter->T2S(source);
    default:
        return source;
    }
    // Just in case
    qDebug("Global::convert: we should not be here");
    return source;
}

} // namespace QTerm

#include <qtermglobal.moc>
