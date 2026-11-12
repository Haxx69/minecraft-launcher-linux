#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSignalBlocker>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QSpinBox>
#include <QSet>
#include <QMap>
#include <QVector>
#include <QAbstractItemView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QProcess>
#include <QProcessEnvironment>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QScrollArea>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include <QUuid>
#include <QFileDialog>
#include <QDirIterator>
#include <functional>
#include <algorithm>

static QString appRoot(){ return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation); }
static QString safeName(QString s){ s=s.trimmed(); s.replace(QRegularExpression("[^A-Za-z0-9._ -]"), "_"); s.replace(' ', '-'); return s.isEmpty()?"Instance":s.left(48); }
static QString sha1(const QByteArray &b){ return QCryptographicHash::hash(b,QCryptographicHash::Sha1).toHex(); }

class LauncherWindow : public QMainWindow {
    Q_OBJECT
public:
    LauncherWindow(){
        setWindowTitle("OfficialCraft 3.4.0 • Minecraft Launcher"); resize(1100,720); setMinimumSize(850,580);
        net=new QNetworkAccessManager(this);
        QDir().mkpath(instancesDir()); QDir().mkpath(configDir());
        buildUi(); loadInstances(); loadManifest();
    }
private:
    QNetworkAccessManager *net{}; QListWidget *instanceList{}; QComboBox *versionBox{}; QLabel *versionStatus{}; QLabel *versionsSummary{}; QCheckBox *lockVersionCheck{}; QLineEdit *javaEdit{}; QPushButton *javaAutoButton{}; QPushButton *javaBrowseButton{}; QLineEdit *ramEdit{}; QLineEdit *playerEdit{}; QProgressBar *progress{}; QLabel *status{}; QPlainTextEdit *log{}; QPushButton *installBtn{}; QPushButton *launchBtn{}; QTableWidget *queueTable{}; QSpinBox *workerSpin{}; QTabWidget *tabs{}; QComboBox *forgeVersionBox{}; QLabel *forgeStatus{}; QLineEdit *modrinthSearchEdit{}; QComboBox *modLoaderBox{}; QTableWidget *modrinthResultsTable{}; QTableWidget *installedModsTable{}; QPushButton *forgeInstallButton{}; bool forgeInstallRunning=false; int queueTotal=0; int queueFinished=0;
    QJsonObject manifest; QString selected;
    QString configDir() const { return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation); }
    QString instancesDir() const { return appRoot()+"/instances"; }
    QString instancePath() const { return instancesDir()+"/"+selected; }
    void note(const QString &s){ log->appendPlainText("["+QDateTime::currentDateTime().toString("HH:mm:ss")+"] "+s); status->setText(s); QApplication::processEvents(); }
    void buildUi(){
        setStyleSheet(R"(
            QWidget{background:#111318;color:#e8eaf0;font-family:'Noto Sans','Ubuntu';font-size:10pt}
            QFrame#sidebar{background:#171a21;border-right:1px solid #292e39} QLabel#brand{font-size:22pt;font-weight:800;color:#f4f6fb} QLabel#muted{color:#969eae}
            QPushButton{background:#252b36;border:1px solid #343c4b;border-radius:8px;padding:10px 15px;font-weight:600} QPushButton:hover{background:#30394a} QPushButton:disabled{color:#687080;background:#1c2028}
            QPushButton#primary{background:#6d5dfc;border:0;color:white} QPushButton#primary:hover{background:#8073ff}
            QListWidget,QComboBox,QLineEdit{background:#1a1e27;border:1px solid #343b49;border-radius:7px;padding:8px;selection-background-color:#6254df}
            QListWidget::item{padding:11px;border-radius:6px} QListWidget::item:selected{background:#302b59;color:white}
            QProgressBar{background:#202530;border:0;border-radius:6px;height:12px;text-align:center} QProgressBar::chunk{background:#7567ff;border-radius:6px}
            QPlainTextEdit{background:#0c0e12;border:1px solid #292e39;border-radius:8px;color:#b9c4d8;font-family:monospace;font-size:9pt}
            QTabWidget::pane{border:1px solid #292e39;border-radius:9px;background:#111318}
            QTabBar::tab{background:#1a1e27;color:#aeb6c5;padding:10px 16px;margin-right:3px;border-top-left-radius:7px;border-top-right-radius:7px}
            QTabBar::tab:selected{background:#302b59;color:white}
            QTableWidget{background:#151922;alternate-background-color:#191f2a;gridline-color:#2a3040;border:1px solid #303848;border-radius:7px}
            QHeaderView::section{background:#202633;color:#dce2ef;padding:8px;border:0}
        )");
        auto *central=new QWidget; setCentralWidget(central); auto *root=new QHBoxLayout(central); root->setContentsMargins(0,0,0,0); root->setSpacing(0);
        auto *side=new QFrame; side->setObjectName("sidebar"); side->setFixedWidth(260); auto *sl=new QVBoxLayout(side); sl->setContentsMargins(18,22,18,18);
        auto *brand=new QLabel("OFFICIALCRAFT"); brand->setObjectName("brand"); sl->addWidget(brand); auto *sub=new QLabel("JAVA EDITION • LINUX"); sub->setObjectName("muted"); sl->addWidget(sub); sl->addSpacing(28);
        sl->addWidget(new QLabel("YOUR INSTANCES")); instanceList=new QListWidget; sl->addWidget(instanceList,1);
        auto *row=new QHBoxLayout; auto *add=new QPushButton("＋ New"); auto *del=new QPushButton("Delete"); row->addWidget(add); row->addWidget(del); sl->addLayout(row); root->addWidget(side);
        auto *main=new QWidget; auto *outer=new QVBoxLayout(main); outer->setContentsMargins(14,14,14,14); root->addWidget(main,1);
        tabs=new QTabWidget; tabs->setDocumentMode(true); outer->addWidget(tabs);
        auto *dashboard=new QWidget; auto *ml=new QVBoxLayout(dashboard); ml->setContentsMargins(18,16,18,14); ml->setSpacing(12); tabs->addTab(dashboard,"Dashboard");
        auto *top=new QHBoxLayout; auto *title=new QLabel("Play Minecraft"); title->setStyleSheet("font-size:25pt;font-weight:750"); top->addWidget(title); top->addStretch(); auto *folder=new QPushButton("Open data folder"); top->addWidget(folder); ml->addLayout(top);
        auto *card=new QFrame; card->setStyleSheet("QFrame{background:#191e27;border:1px solid #2b3240;border-radius:12px}"); auto *grid=new QGridLayout(card); grid->setContentsMargins(20,18,20,18); grid->setHorizontalSpacing(14); grid->setVerticalSpacing(12);
        grid->addWidget(new QLabel("Minecraft version"),0,0); versionBox=new QComboBox; versionBox->setMinimumWidth(220); grid->addWidget(versionBox,0,1);
        versionStatus=new QLabel("Installed version: checking…"); versionStatus->setObjectName("muted"); grid->addWidget(versionStatus,0,2);
        lockVersionCheck=new QCheckBox("Lock version for this instance"); grid->addWidget(lockVersionCheck,0,3);
        grid->addWidget(new QLabel("Java runtime"),1,0); javaEdit=new QLineEdit("auto"); javaEdit->setPlaceholderText("auto (recommended) or /path/to/java"); grid->addWidget(javaEdit,1,1);
        auto *javaButtons=new QWidget; auto *javaButtonsLayout=new QHBoxLayout(javaButtons); javaButtonsLayout->setContentsMargins(0,0,0,0); javaAutoButton=new QPushButton("Auto-detect"); javaBrowseButton=new QPushButton("Browse…"); javaButtonsLayout->addWidget(javaAutoButton); javaButtonsLayout->addWidget(javaBrowseButton); grid->addWidget(javaButtons,1,2,1,2);
        grid->addWidget(new QLabel("Memory (MB)"),2,0); ramEdit=new QLineEdit("2048"); grid->addWidget(ramEdit,2,1);
        grid->addWidget(new QLabel("Offline player name"),3,0); playerEdit=new QLineEdit("Player"); grid->addWidget(playerEdit,3,1); ml->addWidget(card);
        auto *buttons=new QHBoxLayout; installBtn=new QPushButton("Download / Repair files"); installBtn->setObjectName("primary"); launchBtn=new QPushButton("Launch offline"); buttons->addWidget(installBtn); buttons->addWidget(launchBtn); buttons->addStretch(); ml->addLayout(buttons);
        progress=new QProgressBar; progress->setRange(0,100); progress->setValue(0); ml->addWidget(progress); status=new QLabel("Ready"); status->setObjectName("muted"); ml->addWidget(status);
        auto *consolePage=new QWidget; auto *consoleLayout=new QVBoxLayout(consolePage);
        auto *logTitle=new QLabel("LIVE LAUNCHER / GAME LOG"); logTitle->setObjectName("muted"); consoleLayout->addWidget(logTitle);
        log=new QPlainTextEdit; log->setReadOnly(true); consoleLayout->addWidget(log,1); tabs->addTab(consolePage,"Console");
        // Dedicated download queue with one row per file.
        auto *downloadsPage=new QWidget; auto *dl=new QVBoxLayout(downloadsPage);
        auto *dtitle=new QLabel("Download manager"); dtitle->setStyleSheet("font-size:20pt;font-weight:700"); dl->addWidget(dtitle);
        auto *dsub=new QLabel("Parallel transfers • SHA-1 verification • per-file status"); dsub->setObjectName("muted"); dl->addWidget(dsub);
        queueTable=new QTableWidget(0,6); queueTable->setHorizontalHeaderLabels({"File","Status","Progress","Speed","ETA","Destination"});
        queueTable->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
        queueTable->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
        queueTable->horizontalHeader()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
        queueTable->horizontalHeader()->setSectionResizeMode(3,QHeaderView::ResizeToContents);
        queueTable->horizontalHeader()->setSectionResizeMode(4,QHeaderView::ResizeToContents);
        queueTable->horizontalHeader()->setSectionResizeMode(5,QHeaderView::Stretch);
        queueTable->verticalHeader()->setVisible(false); queueTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        queueTable->setSelectionBehavior(QAbstractItemView::SelectRows); dl->addWidget(queueTable,1);
        auto *qrow=new QHBoxLayout; auto *clearQueue=new QPushButton("Clear finished"); qrow->addWidget(clearQueue); qrow->addStretch();
        auto *qinfo=new QLabel("Active transfers are limited by Settings."); qinfo->setObjectName("muted"); qrow->addWidget(qinfo); dl->addLayout(qrow);
        connect(clearQueue,&QPushButton::clicked,this,[this]{for(int r=queueTable->rowCount()-1;r>=0;--r){auto *it=queueTable->item(r,1);if(it && (it->text()=="Completed"||it->text()=="Failed"||it->text()=="Skipped"))queueTable->removeRow(r);}});
        tabs->addTab(downloadsPage,"Downloads");

        auto *versionsPage=new QWidget; auto *vl=new QVBoxLayout(versionsPage);
        auto *vtitle=new QLabel("Minecraft versions"); vtitle->setStyleSheet("font-size:20pt;font-weight:700"); vl->addWidget(vtitle);
        auto *vdesc=new QLabel("Versions are loaded from Mojang's official version manifest. Choose a version on Dashboard, then select Download / Repair files."); vdesc->setWordWrap(true); vdesc->setObjectName("muted"); vl->addWidget(vdesc);
        versionsSummary=new QLabel("Installed in this instance: none"); versionsSummary->setWordWrap(true); versionsSummary->setStyleSheet("background:#191e27;border:1px solid #2b3240;border-radius:8px;padding:12px;color:#cbd5e1;"); vl->addWidget(versionsSummary);
        auto *refreshManifest=new QPushButton("Refresh official version list"); vl->addWidget(refreshManifest); vl->addStretch();
        connect(refreshManifest,&QPushButton::clicked,this,[this]{loadManifest();}); tabs->addTab(versionsPage,"Versions");

        auto *modsPage=new QScrollArea; modsPage->setWidgetResizable(true); modsPage->setFrameShape(QFrame::NoFrame);
        auto *modsContent=new QWidget; auto *modsLayout=new QVBoxLayout(modsContent); modsLayout->setContentsMargins(10,10,10,10);
        auto *mtitle=new QLabel("Forge & Mod Browser"); mtitle->setStyleSheet("font-size:20pt;font-weight:700"); modsLayout->addWidget(mtitle);
        auto *forgeCard=new QFrame; forgeCard->setStyleSheet("QFrame{background:#191e27;border:1px solid #2b3240;border-radius:10px}"); auto *forgeLayout=new QVBoxLayout(forgeCard);
        auto *forgeTitle=new QLabel("Minecraft Forge installer"); forgeTitle->setStyleSheet("font-size:14pt;font-weight:700;border:0"); forgeLayout->addWidget(forgeTitle);
        auto *forgeInfo=new QLabel("Install the official Forge client for the Minecraft version selected on Dashboard. The matching vanilla version must be installed first."); forgeInfo->setWordWrap(true); forgeInfo->setObjectName("muted"); forgeLayout->addWidget(forgeInfo);
        auto *forgeRow=new QHBoxLayout; forgeVersionBox=new QComboBox; forgeVersionBox->setMinimumWidth(250); auto *refreshForge=new QPushButton("Find Forge builds"); forgeInstallButton=new QPushButton("Install Forge"); forgeInstallButton->setObjectName("primary"); forgeRow->addWidget(new QLabel("Forge build")); forgeRow->addWidget(forgeVersionBox,1); forgeRow->addWidget(refreshForge); forgeRow->addWidget(forgeInstallButton); forgeLayout->addLayout(forgeRow);
        forgeStatus=new QLabel("Choose a Minecraft version on Dashboard, then find available Forge builds."); forgeStatus->setWordWrap(true); forgeStatus->setObjectName("muted"); forgeLayout->addWidget(forgeStatus); modsLayout->addWidget(forgeCard);

        auto *browserCard=new QFrame; browserCard->setStyleSheet("QFrame{background:#191e27;border:1px solid #2b3240;border-radius:10px}"); auto *browserLayout=new QVBoxLayout(browserCard);
        auto *browserTitle=new QLabel("Modrinth mod browser"); browserTitle->setStyleSheet("font-size:14pt;font-weight:700;border:0"); browserLayout->addWidget(browserTitle);
        auto *browserInfo=new QLabel("Browse and install mods without an API key. Results are filtered by the selected Minecraft version and mod loader. Select a Forge instance on Dashboard before installing Forge mods."); browserInfo->setWordWrap(true); browserInfo->setObjectName("muted"); browserLayout->addWidget(browserInfo);
        auto *searchRow=new QHBoxLayout; modrinthSearchEdit=new QLineEdit; modrinthSearchEdit->setPlaceholderText("Search mods (e.g. JEI, JourneyMap)"); modLoaderBox=new QComboBox; modLoaderBox->addItems({"Forge"}); auto *searchMods=new QPushButton("Search Modrinth"); searchMods->setObjectName("primary"); searchRow->addWidget(modrinthSearchEdit,1); searchRow->addWidget(modLoaderBox); searchRow->addWidget(searchMods); browserLayout->addLayout(searchRow);
        modrinthResultsTable=new QTableWidget(0,4); modrinthResultsTable->setHorizontalHeaderLabels({"Mod","Author","Downloads","Project ID"}); modrinthResultsTable->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch); modrinthResultsTable->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents); modrinthResultsTable->horizontalHeader()->setSectionResizeMode(2,QHeaderView::ResizeToContents); modrinthResultsTable->horizontalHeader()->setSectionResizeMode(3,QHeaderView::ResizeToContents); modrinthResultsTable->verticalHeader()->setVisible(false); modrinthResultsTable->setSelectionBehavior(QAbstractItemView::SelectRows); modrinthResultsTable->setSelectionMode(QAbstractItemView::SingleSelection); modrinthResultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); modrinthResultsTable->setMaximumHeight(240); browserLayout->addWidget(modrinthResultsTable);
        auto *installMod=new QPushButton("Install selected mod into this instance"); installMod->setObjectName("primary"); browserLayout->addWidget(installMod); modsLayout->addWidget(browserCard);

        auto *localCard=new QFrame; localCard->setStyleSheet("QFrame{background:#191e27;border:1px solid #2b3240;border-radius:10px}"); auto *localLayout=new QVBoxLayout(localCard);
        auto *localTitle=new QLabel("Installed mods in this instance"); localTitle->setStyleSheet("font-size:14pt;font-weight:700;border:0"); localLayout->addWidget(localTitle);
        auto *localInfo=new QLabel("Manage JAR files in this instance's game/mods folder. Disabled mods use the .jar.disabled suffix."); localInfo->setWordWrap(true); localInfo->setObjectName("muted"); localLayout->addWidget(localInfo);
        installedModsTable=new QTableWidget(0,3); installedModsTable->setHorizontalHeaderLabels({"File","State","Size"}); installedModsTable->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch); installedModsTable->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents); installedModsTable->horizontalHeader()->setSectionResizeMode(2,QHeaderView::ResizeToContents); installedModsTable->verticalHeader()->setVisible(false); installedModsTable->setSelectionBehavior(QAbstractItemView::SelectRows); installedModsTable->setSelectionMode(QAbstractItemView::SingleSelection); installedModsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); installedModsTable->setMaximumHeight(210); localLayout->addWidget(installedModsTable);
        auto *localButtons=new QHBoxLayout; auto *importLocal=new QPushButton("Add local JAR"); auto *toggleLocal=new QPushButton("Enable / Disable"); auto *removeLocal=new QPushButton("Remove"); auto *refreshLocal=new QPushButton("Refresh"); auto *openLocal=new QPushButton("Open mods folder"); localButtons->addWidget(importLocal); localButtons->addWidget(toggleLocal); localButtons->addWidget(removeLocal); localButtons->addWidget(refreshLocal); localButtons->addWidget(openLocal); localLayout->addLayout(localButtons);
        modsLayout->addWidget(localCard); modsLayout->addStretch();
        modsPage->setWidget(modsContent); tabs->addTab(modsPage,"Forge & Mods");
        // Remove legacy CurseForge credentials from launcher settings; Modrinth is keyless.
        QSettings oldSettings(configDir()+"/launcher.ini",QSettings::IniFormat); oldSettings.remove("curseforge/apiKey"); oldSettings.sync();
        connect(refreshForge,&QPushButton::clicked,this,[this]{loadForgeVersions();}); connect(forgeInstallButton,&QPushButton::clicked,this,[this]{installForgeClient();}); connect(searchMods,&QPushButton::clicked,this,[this]{searchModrinth();}); connect(modrinthSearchEdit,&QLineEdit::returnPressed,this,[this]{searchModrinth();}); connect(installMod,&QPushButton::clicked,this,[this]{installSelectedModrinthMod();});
        connect(importLocal,&QPushButton::clicked,this,[this]{importLocalMod();});
        connect(toggleLocal,&QPushButton::clicked,this,[this]{toggleSelectedMod();});
        connect(removeLocal,&QPushButton::clicked,this,[this]{removeSelectedMod();});
        connect(refreshLocal,&QPushButton::clicked,this,[this]{refreshInstalledMods();});
        connect(openLocal,&QPushButton::clicked,this,[this]{QDir().mkpath(instancePath()+"/game/mods");QDesktopServices::openUrl(QUrl::fromLocalFile(instancePath()+"/game/mods"));});
        connect(instanceList,&QListWidget::currentTextChanged,this,[this](const QString &name){Q_UNUSED(name);QTimer::singleShot(0,this,[this]{refreshInstalledMods();});});
        refreshInstalledMods();
        auto *accountsPage=new QWidget; auto *al=new QVBoxLayout(accountsPage);
        auto *atitle=new QLabel("Accounts"); atitle->setStyleSheet("font-size:20pt;font-weight:700"); al->addWidget(atitle);
        auto *offlineInfo=new QLabel("Offline profiles are for local single-player and LAN use only. Microsoft sign-in is not implemented in this build; authenticated multiplayer and Realms are unavailable."); offlineInfo->setWordWrap(true); offlineInfo->setObjectName("muted"); al->addWidget(offlineInfo);
        auto *accountFolder=new QPushButton("Open account / configuration folder"); al->addWidget(accountFolder);
        connect(accountFolder,&QPushButton::clicked,this,[this]{QDesktopServices::openUrl(QUrl::fromLocalFile(configDir()));});
        al->addStretch(); tabs->addTab(accountsPage,"Accounts");

        auto *settingsPage=new QWidget; auto *slayout=new QVBoxLayout(settingsPage);
        auto *stitle=new QLabel("Launcher settings"); stitle->setStyleSheet("font-size:20pt;font-weight:700"); slayout->addWidget(stitle);
        auto *workersRow=new QHBoxLayout; workersRow->addWidget(new QLabel("Parallel download workers")); workerSpin=new QSpinBox; workerSpin->setRange(1,16); workerSpin->setValue(6); workersRow->addWidget(workerSpin); workersRow->addStretch(); slayout->addLayout(workersRow);
        auto *saveWorkers=new QPushButton("Save settings"); slayout->addWidget(saveWorkers);
        connect(saveWorkers,&QPushButton::clicked,this,[this]{QSettings set(configDir()+"/launcher.ini",QSettings::IniFormat);set.setValue("downloadWorkers",workerSpin->value());note("Saved launcher settings.");});
        QSettings savedSettings(configDir()+"/launcher.ini",QSettings::IniFormat); workerSpin->setValue(savedSettings.value("downloadWorkers",6).toInt());
        slayout->addStretch(); tabs->addTab(settingsPage,"Settings");

        connect(add,&QPushButton::clicked,this,[this]{createInstance();}); connect(del,&QPushButton::clicked,this,[this]{deleteInstance();});
        connect(instanceList,&QListWidget::currentTextChanged,this,[this](const QString &s){ if(!s.isEmpty()){selected=s; loadInstanceSettings(); saveGlobal();}});
        connect(installBtn,&QPushButton::clicked,this,[this]{installVersion();}); connect(launchBtn,&QPushButton::clicked,this,[this]{launchOffline();});
        connect(folder,&QPushButton::clicked,this,[this]{QDir().mkpath(instancePath()); QDesktopServices::openUrl(QUrl::fromLocalFile(instancePath()));});
        connect(versionBox,&QComboBox::currentTextChanged,this,[this](const QString&){saveInstanceSettings();refreshVersionStatus();});
        connect(lockVersionCheck,&QCheckBox::toggled,this,[this](bool locked){versionBox->setEnabled(!locked);saveInstanceSettings();});
        connect(javaEdit,&QLineEdit::editingFinished,this,[this]{saveInstanceSettings();});
        connect(javaAutoButton,&QPushButton::clicked,this,[this]{autoDetectJava();});
        connect(javaBrowseButton,&QPushButton::clicked,this,[this]{QString f=QFileDialog::getOpenFileName(this,"Select Java executable",javaEdit->text().startsWith("/")?javaEdit->text():QDir::homePath(),"Java executable (java)");if(!f.isEmpty()){javaEdit->setText(QFileInfo(f).canonicalFilePath().isEmpty()?f:QFileInfo(f).canonicalFilePath());saveInstanceSettings();autoDetectJava(true);}}); connect(ramEdit,&QLineEdit::editingFinished,this,[this]{saveInstanceSettings();}); connect(playerEdit,&QLineEdit::editingFinished,this,[this]{saveInstanceSettings();});
    }
    void loadInstances(){
        instanceList->clear(); QDir d(instancesDir()); const auto dirs=d.entryList(QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name);
        for(const auto &n:dirs) instanceList->addItem(n);
        QSettings s(configDir()+"/launcher.ini",QSettings::IniFormat); QString last=s.value("selectedInstance").toString();
        if(instanceList->count()==0){createInstance("Default");return;}
        for(int i=0;i<instanceList->count();++i) if(instanceList->item(i)->text()==last){instanceList->setCurrentRow(i);return;}
        instanceList->setCurrentRow(0);
    }
    void createInstance(QString suggested={}){
        bool ok=true; QString n=suggested;
        if(n.isEmpty()) n=QInputDialog::getText(this,"Create instance","Instance name:",QLineEdit::Normal,"My Minecraft",&ok);
        if(!ok||n.trimmed().isEmpty()) return; n=safeName(n); if(QDir(instancesDir()+"/"+n).exists()){QMessageBox::warning(this,"Already exists","An instance with that name already exists.");return;}
        QString p=instancesDir()+"/"+n; for(QString sub:{"game/saves","game/mods","game/resourcepacks","game/shaderpacks","versions","libraries","assets/indexes","assets/objects","natives","logs"}) QDir().mkpath(p+"/"+sub);
        QJsonObject cfg{{"name",n},{"version",""},{"java","auto"},{"memory",2048},{"player","Player"}}; QFile f(p+"/instance.json"); if(f.open(QIODevice::WriteOnly)) f.write(QJsonDocument(cfg).toJson(QJsonDocument::Indented));
        instanceList->addItem(n); instanceList->setCurrentRow(instanceList->count()-1); note("Created instance: "+n);
    }
    void deleteInstance(){ if(selected.isEmpty())return; if(QMessageBox::question(this,"Delete instance","Permanently delete '"+selected+"' and all its worlds/mods?")!=QMessageBox::Yes)return; QDir(instancePath()).removeRecursively(); selected.clear(); loadInstances(); }
    QJsonObject readInstance() const {
        QFile f(instancePath()+"/instance.json");
        if(!f.open(QIODevice::ReadOnly)) return {};
        return QJsonDocument::fromJson(f.readAll()).object();
    }
    bool writeInstance(const QJsonObject &cfg) const {
        QSaveFile f(instancePath()+"/instance.json");
        if(!f.open(QIODevice::WriteOnly)) return false;
        f.write(QJsonDocument(cfg).toJson(QJsonDocument::Indented));
        return f.commit();
    }
    QJsonObject readVersionMetadata(const QString &id) const {
        QFile f(instancePath()+"/versions/"+id+"/"+id+".json");
        if(f.open(QIODevice::ReadOnly)) return QJsonDocument::fromJson(f.readAll()).object();
        // Backward compatibility with 3.1.2 installations.
        QJsonObject cfg=readInstance();
        if(cfg.value("version").toString()==id) return cfg.value("versionJson").toObject();
        return {};
    }
    QString selectedMinecraftBaseVersion() const {
        const QString id=versionBox?versionBox->currentText().trimmed():QString();
        if(id.isEmpty()) return {};
        const QJsonObject metadata=readVersionMetadata(id);
        const QString parent=metadata.value("inheritsFrom").toString().trimmed();
        return parent.isEmpty()?id:parent;
    }
    QStringList installedVersions() const {
        QStringList found;
        QDir versions(instancePath()+"/versions");
        if(!versions.exists()) return found;
        const QStringList dirs=versions.entryList(QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name);
        const QJsonObject cfg=readInstance();
        for(const QString &id:dirs) {
            const QString jar=versions.filePath(id+"/"+id+".jar");
            const QString metadata=versions.filePath(id+"/"+id+".json");
            const bool legacyMetadata=(cfg.value("version").toString()==id && !cfg.value("versionJson").toObject().isEmpty());
            const bool validMetadata=QFileInfo::exists(metadata) || legacyMetadata;
            bool validJar=QFileInfo::exists(jar) && QFileInfo(jar).size()>0;
            if(validMetadata && !validJar){QJsonObject m=readVersionMetadata(id);QString parent=m.value("inheritsFrom").toString();if(!parent.isEmpty()){QString parentJar=versions.filePath(parent+"/"+parent+".jar");validJar=QFileInfo::exists(parentJar)&&QFileInfo(parentJar).size()>0;}}
            if(validMetadata && validJar) found << id;
        }
        return found;
    }
    void refreshVersionStatus() {
        if(!versionStatus || selected.isEmpty()) return;
        const QString id=versionBox->currentText();
        const QStringList installed=installedVersions();
        if(id.isEmpty()) versionStatus->setText("No version selected");
        else if(installed.contains(id)) versionStatus->setText("✓ "+id+" is installed in this instance");
        else versionStatus->setText("Not installed in this instance");
        if(versionsSummary) versionsSummary->setText(installed.isEmpty() ? "Installed in this instance: none" : "Installed in this instance: "+installed.join(", "));
    }
    struct JavaRuntime {
        QString path;
        QString version;
        int major = 0;
        bool valid = false;
    };
    QString normalizedJavaPath(const QString &candidate) const {
        QString c=candidate.trimmed();
        if(c.isEmpty() || c.compare("auto",Qt::CaseInsensitive)==0) return {};
        if(c.contains('/')) {
            QFileInfo fi(c);
            if(!fi.exists() || !fi.isFile() || !fi.isExecutable()) return {};
            QString canonical=fi.canonicalFilePath();
            return canonical.isEmpty()?fi.absoluteFilePath():canonical;
        }
        return QStandardPaths::findExecutable(c);
    }
    JavaRuntime inspectJava(const QString &candidate) const {
        JavaRuntime info;
        const QString path=normalizedJavaPath(candidate);
        if(path.isEmpty()) return info;
        QProcess proc;
        QProcessEnvironment env=QProcessEnvironment::systemEnvironment();
        env.remove("JAVA_TOOL_OPTIONS"); env.remove("_JAVA_OPTIONS"); env.remove("JDK_JAVA_OPTIONS");
        proc.setProcessEnvironment(env);
        proc.start(path,{"-version"});
        if(!proc.waitForStarted(3000) || !proc.waitForFinished(5000)) { proc.kill(); proc.waitForFinished(1000); return info; }
        const QString output=QString::fromLocal8Bit(proc.readAllStandardOutput()+proc.readAllStandardError());
        QRegularExpression re("(?:version\\s+)?\\\"?([0-9]+)(?:\\.([0-9]+))?");
        QRegularExpressionMatch m=re.match(output);
        if(!m.hasMatch()) return info;
        int first=m.captured(1).toInt(); int second=m.captured(2).toInt();
        info.major=(first==1 && second>0)?second:first;
        info.path=path; info.version=output.trimmed().section('\n',0,0); info.valid=info.major>0 && proc.exitStatus()==QProcess::NormalExit;
        return info;
    }
    QStringList javaCandidatePaths() const {
        QStringList paths;
        auto add=[&](const QString &p){if(!p.isEmpty()&&!paths.contains(p))paths<<p;};
        add(normalizedJavaPath(javaEdit?javaEdit->text():QString()));
        add(QStandardPaths::findExecutable("java"));
        const QString javaHome=qEnvironmentVariable("JAVA_HOME"); if(!javaHome.isEmpty()) add(javaHome+"/bin/java");
        const QStringList roots={"/usr/lib/jvm","/usr/java","/usr/local/java","/opt/java","/opt/jdk",QDir::homePath()+"/.jdks",QDir::homePath()+"/.sdkman/candidates/java",QDir::homePath()+"/.local/share/mise/installs/java"};
        for(const QString &root:roots){
            QDir d(root); if(!d.exists()) continue;
            const QStringList dirs=d.entryList(QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name);
            for(const QString &name:dirs) add(d.filePath(name+"/bin/java"));
            add(d.filePath("bin/java"));
        }
        return paths;
    }
    QList<JavaRuntime> discoverJavaRuntimes() const {
        QList<JavaRuntime> found;
        for(const QString &candidate:javaCandidatePaths()){
            JavaRuntime runtime=inspectJava(candidate);
            if(!runtime.valid) continue;
            bool duplicate=false; for(const JavaRuntime &old:found) if(old.path==runtime.path) duplicate=true;
            if(!duplicate) found.append(runtime);
        }
        std::sort(found.begin(),found.end(),[](const JavaRuntime &a,const JavaRuntime &b){if(a.major!=b.major)return a.major<b.major;return a.path<b.path;});
        return found;
    }
    int requiredJavaMajor(const QJsonObject &metadata,const QString &versionId) const {
        int required=metadata.value("javaVersion").toObject().value("majorVersion").toInt(0);
        // Some version metadata carries a JVM option introduced in JDK 23.
        const QJsonArray jvm=metadata.value("arguments").toObject().value("jvm").toArray();
        for(const QJsonValue &v:jvm){
            QStringList args; if(v.isString())args<<v.toString(); else if(v.isObject()){QJsonValue value=v.toObject().value("value");if(value.isString())args<<value.toString();else if(value.isArray())for(const QJsonValue &x:value.toArray())if(x.isString())args<<x.toString();}
            for(const QString &arg:args) if(arg.contains("--sun-misc-unsafe-memory-access")) required=qMax(required,23);
        }
        if(required>0)return required;
        QRegularExpression vr("^(\\d+)\\.(\\d+)(?:\\.(\\d+))?"); auto m=vr.match(versionId);
        if(m.hasMatch()){
            int major=m.captured(1).toInt(),minor=m.captured(2).toInt(),patch=m.captured(3).toInt();
            if(major==1 && minor<=16)return 8;
            if(major==1 && minor==17)return 16;
            if(major==1 && minor>=18 && minor<20)return 17;
            if(major==1 && minor==20 && patch<5)return 17;
            if(major==1 && minor>=20)return 21;
            if(major>=2)return 21;
        }
        return 0;
    }
    QString chooseJavaRuntime(const QJsonObject &metadata,const QString &versionId,bool showError=true) {
        const int required=requiredJavaMajor(metadata,versionId);
        const QList<JavaRuntime> runtimes=discoverJavaRuntimes();
        const QString configured=javaEdit->text().trimmed();
        const QString configuredPath=normalizedJavaPath(configured);
        JavaRuntime configuredInfo;
        for(const JavaRuntime &r:runtimes) if(r.path==configuredPath) configuredInfo=r;
        // A manually selected runtime is honored only when it meets the metadata requirement.
        if(configuredInfo.valid && (required==0 || configuredInfo.major==required)) return configuredInfo.path;
        // Prefer an exact major match to avoid old/new Java compatibility surprises.
        for(const JavaRuntime &r:runtimes) if(required>0 && r.major==required) return r.path;
        // If exact runtime is absent, a newer runtime is a fallback; never select an older one.
        for(const JavaRuntime &r:runtimes) if(required>0 && r.major>required) return r.path;
        if(required==0 && !runtimes.isEmpty()){
            for(const JavaRuntime &r:runtimes) if(r.path==configuredPath)return r.path;
            return runtimes.last().path;
        }
        if(showError){
            QStringList detected; for(const JavaRuntime &r:runtimes) detected<<QString("Java %1 — %2").arg(r.major).arg(r.path);
            QString message=required>0?QString("Minecraft %1 requires Java %2 or newer.\n\n").arg(versionId).arg(required):QString("No usable Java runtime was found.\n\n");
            message+="Detected runtimes:\n"+(detected.isEmpty()?QString("  (none)\n"):detected.join("\n")+"\n");
            message+="\nInstall the required OpenJDK runtime using your distribution's package manager, then click Auto-detect. You can also browse to a Java executable manually.";
            QMessageBox::warning(this,"Compatible Java runtime not found",message);
            note("Java runtime search found no compatible runtime for Minecraft "+versionId+".");
        }
        return {};
    }
    void autoDetectJava(bool quiet=false) {
        if(selected.isEmpty())return;
        const QString id=versionBox->currentText();
        QJsonObject metadata=readVersionMetadata(id);
        const QString parent=metadata.value("inheritsFrom").toString();
        if(!parent.isEmpty()){
            QJsonObject base=readVersionMetadata(parent);
            if(metadata.value("javaVersion").isUndefined())metadata.insert("javaVersion",base.value("javaVersion"));
            QJsonObject args=metadata.value("arguments").toObject(); if(args.value("jvm").toArray().isEmpty())args.insert("jvm",base.value("arguments").toObject().value("jvm")); metadata.insert("arguments",args);
        }
        const QString chosen=chooseJavaRuntime(metadata,id,!quiet);
        if(!chosen.isEmpty()){
            javaEdit->setText(chosen);saveInstanceSettings();
            if(!quiet)note("Selected Java runtime: "+chosen);
        }
    }
    void saveInstanceSettings() {
        if(selected.isEmpty() || !versionBox) return;
        QJsonObject c=readInstance();
        c["name"]=selected;
        c["version"]=versionBox->currentText();
        c["versionLocked"]=lockVersionCheck && lockVersionCheck->isChecked();
        c["java"]=javaEdit->text();
        c["memory"]=ramEdit->text().toInt();
        c["player"]=playerEdit->text();
        writeInstance(c);
    }
    void loadInstanceSettings() {
        if(selected.isEmpty()) return;
        const QJsonObject c=readInstance();
        javaEdit->setText(c.value("java").toString("auto"));
        ramEdit->setText(QString::number(c.value("memory").toInt(2048)));
        playerEdit->setText(c.value("player").toString("Player"));
        const QString wanted=c.value("version").toString();
        bool selectedDefaultVersion=false;
        {
            const QSignalBlocker blocker(versionBox);
            versionBox->clear();
            for(const QJsonValue &entry:manifest.value("versions").toArray()){
                const QJsonObject item=entry.toObject();
                if(item.value("type").toString()=="release") versionBox->addItem(item.value("id").toString());
            }
            QDir localVersions(instancePath()+"/versions");
            for(const QString &localId:localVersions.entryList(QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name)){
                const QString localMeta=localVersions.filePath(localId+"/"+localId+".json");
                if(QFileInfo::exists(localMeta)&&versionBox->findText(localId)<0)versionBox->addItem(localId);
            }
            const int i=versionBox->findText(wanted);
            if(i>=0) versionBox->setCurrentIndex(i);
            else if(versionBox->count()>0 && wanted.isEmpty()) { versionBox->setCurrentIndex(0); selectedDefaultVersion=true; }
        }
        const bool locked=c.value("versionLocked").toBool(false);
        { const QSignalBlocker blocker(lockVersionCheck); lockVersionCheck->setChecked(locked); }
        versionBox->setEnabled(!locked);
        // Migrate the previous single-version metadata into its own version folder.
        const QString oldId=c.value("version").toString();
        const QJsonObject oldMetadata=c.value("versionJson").toObject();
        if(!oldId.isEmpty() && !oldMetadata.isEmpty()) {
            const QString path=instancePath()+"/versions/"+oldId+"/"+oldId+".json";
            if(!QFileInfo::exists(path)) { QDir().mkpath(QFileInfo(path).absolutePath()); QFile f(path); if(f.open(QIODevice::WriteOnly)) f.write(QJsonDocument(oldMetadata).toJson(QJsonDocument::Indented)); }
        }
        saveGlobal();
        if(selectedDefaultVersion) saveInstanceSettings();
        refreshVersionStatus();
    }
    void saveGlobal(){QSettings s(configDir()+"/launcher.ini",QSettings::IniFormat);s.setValue("selectedInstance",selected);}
    QByteArray request(const QString &url, bool *ok=nullptr){
        QNetworkRequest req{QUrl(url)}; req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::NoLessSafeRedirectPolicy); req.setHeader(QNetworkRequest::UserAgentHeader,"OfficialCraft/3.4");
        QNetworkReply *r=net->get(req); QEventLoop loop; connect(r,&QNetworkReply::finished,&loop,&QEventLoop::quit); QTimer timer; timer.setSingleShot(true); connect(&timer,&QTimer::timeout,r,&QNetworkReply::abort); connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit); timer.start(90000); loop.exec();
        bool good=r->error()==QNetworkReply::NoError && r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()>=200 && r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()<300;
        QByteArray data=r->readAll(); if(!good)note("HTTP download failed: "+url+" — "+r->errorString()); r->deleteLater(); if(ok)*ok=good; return good?data:QByteArray();
    }
    void refreshInstalledMods() {
        if(!installedModsTable || selected.isEmpty()) return;
        QDir().mkpath(instancePath()+"/game/mods");
        QDir mods(instancePath()+"/game/mods");
        const QStringList files=mods.entryList({"*.jar","*.jar.disabled"},QDir::Files,QDir::Name);
        installedModsTable->setRowCount(0);
        for(const QString &file:files) {
            const QFileInfo info(mods.filePath(file));
            const bool enabled=file.endsWith(".jar",Qt::CaseInsensitive);
            const int row=installedModsTable->rowCount();
            installedModsTable->insertRow(row);
            auto *name=new QTableWidgetItem(file); name->setData(Qt::UserRole,file);
            installedModsTable->setItem(row,0,name);
            installedModsTable->setItem(row,1,new QTableWidgetItem(enabled?"Enabled":"Disabled"));
            const qint64 bytes=info.size();
            const QString size=bytes>=1048576?QString::number(bytes/1048576.0,'f',2)+" MB":QString::number(bytes/1024.0,'f',1)+" KB";
            installedModsTable->setItem(row,2,new QTableWidgetItem(size));
        }
    }
    QString selectedModFilename() const {
        if(!installedModsTable) return {};
        const int row=installedModsTable->currentRow();
        if(row<0 || !installedModsTable->item(row,0)) return {};
        return installedModsTable->item(row,0)->data(Qt::UserRole).toString();
    }
    void importLocalMod() {
        if(selected.isEmpty()) return;
        const QString source=QFileDialog::getOpenFileName(this,"Add a Minecraft mod JAR",QDir::homePath(),"Java archives (*.jar)");
        if(source.isEmpty()) return;
        QFile input(source);
        if(!input.open(QIODevice::ReadOnly) || input.read(4)!=QByteArray("PK\x03\x04",4)) {
            QMessageBox::warning(this,"Invalid mod","The selected file is not a valid ZIP/JAR archive.");
            return;
        }
        input.close();
        const QString dir=instancePath()+"/game/mods";
        QDir().mkpath(dir);
        QString name=QFileInfo(source).fileName();
        QString dest=dir+"/"+name;
        if(QFileInfo::exists(dest)) {
            const QString stem=QFileInfo(name).completeBaseName();
            const QString ext=QFileInfo(name).suffix();
            name=stem+"-"+QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss")+"."+ext;
            dest=dir+"/"+name;
        }
        if(!QFile::copy(source,dest)) { QMessageBox::warning(this,"Copy failed","Could not copy the mod into this instance."); return; }
        note("Added local mod: "+name);
        refreshInstalledMods();
    }
    void toggleSelectedMod() {
        const QString name=selectedModFilename();
        if(name.isEmpty()) { QMessageBox::information(this,"Select mod","Select a mod first."); return; }
        const QString from=instancePath()+"/game/mods/"+name;
        QString target;
        if(name.endsWith(".jar.disabled",Qt::CaseInsensitive)) target=from.left(from.size()-QString(".disabled").size());
        else if(name.endsWith(".jar",Qt::CaseInsensitive)) target=from+".disabled";
        else return;
        if(!QFile::rename(from,target)) { QMessageBox::warning(this,"Rename failed","Could not change this mod's enabled state."); return; }
        note(QFileInfo(target).fileName()+(target.endsWith(".disabled")?" disabled.":" enabled."));
        refreshInstalledMods();
    }
    void removeSelectedMod() {
        const QString name=selectedModFilename();
        if(name.isEmpty()) { QMessageBox::information(this,"Select mod","Select a mod first."); return; }
        if(QMessageBox::question(this,"Remove mod","Remove "+name+" from this instance?")!=QMessageBox::Yes) return;
        if(!QFile::remove(instancePath()+"/game/mods/"+name)) { QMessageBox::warning(this,"Remove failed","Could not remove this file."); return; }
        note("Removed mod: "+name);
        refreshInstalledMods();
    }
    QString libraryPathFromCoordinate(const QString &coordinate) const {
        QString c=coordinate, ext="jar"; int at=c.indexOf('@');
        if(at>=0){ext=c.mid(at+1);c=c.left(at);} QStringList parts=c.split(':');
        if(parts.size()<3)return {}; QString group=parts[0]; group.replace('.','/');
        QString file=parts[1]+"-"+parts[2]; if(parts.size()>=4&&!parts[3].isEmpty())file+="-"+parts[3];
        return group+"/"+parts[1]+"/"+parts[2]+"/"+file+"."+ext;
    }
    bool ensureLegacyLauncherProfile(const QString &mc) {
        const QString path=instancePath()+"/launcher_profiles.json";
        QJsonObject root; QFile existing(path);
        if(existing.open(QIODevice::ReadOnly)){root=QJsonDocument::fromJson(existing.readAll()).object();existing.close();}
        QJsonObject profiles=root.value("profiles").toObject();
        QJsonObject profile=profiles.value("OfficialCraft").toObject();
        profile["name"]="OfficialCraft"; profile["lastVersionId"]=mc;
        profile["gameDir"]=instancePath()+"/game"; profile["type"]="custom";
        profile["created"]=profile.value("created").toString(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)+"Z");
        profile["lastUsed"]=QDateTime::currentDateTimeUtc().toString(Qt::ISODate)+"Z";
        profiles["OfficialCraft"]=profile; root["profiles"]=profiles; root["selectedProfile"]="OfficialCraft";
        if(root.value("clientToken").toString().isEmpty())root["clientToken"]=QUuid::createUuid().toString(QUuid::WithoutBraces);
        if(!root.value("authenticationDatabase").isObject())root["authenticationDatabase"]=QJsonObject();
        if(!root.value("selectedUser").isObject())root["selectedUser"]=QJsonObject();
        QSaveFile out(path); if(!out.open(QIODevice::WriteOnly))return false;
        out.write(QJsonDocument(root).toJson(QJsonDocument::Indented)); return out.commit();
    }
    void loadForgeVersions(){
        const QString mc=selectedMinecraftBaseVersion();
        if(mc.isEmpty()){QMessageBox::information(this,"Select version","Choose a Minecraft version on Dashboard first.");return;}
        forgeVersionBox->clear(); forgeStatus->setText("Loading official Forge Maven metadata for "+mc+"…");
        bool ok=false; QByteArray xml=request("https://maven.minecraftforge.net/net/minecraftforge/forge/maven-metadata.xml",&ok);
        if(!ok){forgeStatus->setText("Could not retrieve Forge Maven metadata.");return;}
        QXmlStreamReader reader(xml); QStringList builds;
        while(!reader.atEnd()){reader.readNext();if(reader.isStartElement()&&reader.name()==QStringLiteral("version")){QString v=reader.readElementText().trimmed();if(v.startsWith(mc+"-"))builds<<v;}}
        if(reader.hasError()){forgeStatus->setText("Forge metadata XML error: "+reader.errorString());return;}
        builds.removeDuplicates(); std::sort(builds.begin(),builds.end(),[](const QString&a,const QString&b){return a>b;});
        for(const QString &v:builds)forgeVersionBox->addItem(v);
        forgeStatus->setText(builds.isEmpty()?"No Forge builds found for "+mc:"Found "+QString::number(builds.size())+" Forge builds for "+mc+".");
    }
    void installForgeClient(){
        if(forgeInstallRunning||selected.isEmpty())return;
        const QString mc=selectedMinecraftBaseVersion(),forge=forgeVersionBox->currentText();
        if(mc.isEmpty()||forge.isEmpty()){QMessageBox::information(this,"Select Forge build","Find a Forge build first.");return;}
        const QString vanillaJar=instancePath()+"/versions/"+mc+"/"+mc+".jar";
        if(!QFileInfo::exists(vanillaJar)){int ans=QMessageBox::question(this,"Vanilla required","Install Minecraft "+mc+" in this instance before Forge?");if(ans!=QMessageBox::Yes)return;int ix=versionBox->findText(mc);if(ix<0){QMessageBox::warning(this,"Unavailable","Base Minecraft version is not in the manifest list.");return;}versionBox->setCurrentIndex(ix);if(!installVersion()||!QFileInfo::exists(vanillaJar)){note("Forge setup stopped: vanilla installation did not complete.");return;}}
        if(!ensureLegacyLauncherProfile(mc)){QMessageBox::warning(this,"Profile error","Could not create launcher_profiles.json for the Forge installer.");return;}
        const QString artifact="forge-"+forge+"-installer.jar", url="https://maven.minecraftforge.net/net/minecraftforge/forge/"+forge+"/"+artifact;
        const QString installer=instancePath()+"/cache/forge/"+artifact;
        note("Downloading official Forge installer "+forge+"…"); if(!download(url,installer)){QMessageBox::warning(this,"Download failed","Forge installer download failed. See Console.");return;}
        {QFile check(installer);if(!check.open(QIODevice::ReadOnly)||check.read(4)!=QByteArray("PK\x03\x04",4)){check.close();QFile::remove(installer);QMessageBox::warning(this,"Invalid installer","The Forge installer download is not a valid archive.");return;}}
        forgeInstallRunning=true; if(forgeInstallButton)forgeInstallButton->setEnabled(false);
        auto *proc=new QProcess(this); proc->setWorkingDirectory(instancePath());
        connect(proc,&QProcess::readyReadStandardOutput,this,[this,proc]{QString x=QString::fromLocal8Bit(proc->readAllStandardOutput()).trimmed();if(!x.isEmpty())note("[Forge] "+x);});
        connect(proc,&QProcess::readyReadStandardError,this,[this,proc]{QString x=QString::fromLocal8Bit(proc->readAllStandardError()).trimmed();if(!x.isEmpty())note("[Forge] "+x);});
        connect(proc,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this,proc,mc,forge](int code,QProcess::ExitStatus){
            proc->deleteLater();forgeInstallRunning=false;if(forgeInstallButton)forgeInstallButton->setEnabled(true);
            if(code!=0){forgeStatus->setText("Forge installer failed (exit "+QString::number(code)+"). See Console.");note("Forge installer exited with code "+QString::number(code));return;}
            QDir dirs(instancePath()+"/versions");QStringList matches;
            for(const QString &d:dirs.entryList(QDir::Dirs|QDir::NoDotAndDotDot)){QFile f(dirs.filePath(d+"/"+d+".json"));if(!f.open(QIODevice::ReadOnly))continue;QJsonObject m=QJsonDocument::fromJson(f.readAll()).object();if(m.value("inheritsFrom").toString()==mc&&d.contains("forge",Qt::CaseInsensitive))matches<<d;}
            if(matches.isEmpty()){forgeStatus->setText("Installer exited successfully, but no Forge profile was detected.");note("Forge installer completed without a detected profile.");return;}
            std::sort(matches.begin(),matches.end());QString installed=matches.last();if(versionBox->findText(installed)<0)versionBox->addItem(installed);versionBox->setCurrentText(installed);
            QJsonObject cfg=readInstance();cfg["loader"]="forge";cfg["loaderVersion"]=forge;cfg["version"]=installed;QJsonArray a=cfg.value("installedVersions").toArray();if(!a.contains(installed))a.append(installed);cfg["installedVersions"]=a;writeInstance(cfg);refreshVersionStatus();forgeStatus->setText("Installed Forge "+forge+" for Minecraft "+mc+". Selected profile: "+installed);note("Forge installed successfully: "+installed);
        });
        connect(proc,&QProcess::errorOccurred,this,[this,proc](QProcess::ProcessError e){if(e==QProcess::FailedToStart){forgeInstallRunning=false;if(forgeInstallButton)forgeInstallButton->setEnabled(true);note("Could not start Forge installer: "+proc->errorString());proc->deleteLater();}});
        QJsonObject forgeMeta=readVersionMetadata(mc); QString forgeJava=chooseJavaRuntime(forgeMeta,mc,true); if(forgeJava.isEmpty()){forgeInstallRunning=false;if(forgeInstallButton)forgeInstallButton->setEnabled(true);proc->deleteLater();return;}
        note("Running Forge installer for instance "+selected+" using "+forgeJava+"…");proc->start(forgeJava,{"-Djava.net.preferIPv4Stack=true","-jar",installer,"--installClient",instancePath()});
    }
    QByteArray publicJsonRequest(const QString &url,bool *ok=nullptr){
        QNetworkRequest req{QUrl(url)}; req.setRawHeader("Accept","application/json"); req.setHeader(QNetworkRequest::UserAgentHeader,"OfficialCraft/3.4 (Minecraft launcher)"); req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::NoLessSafeRedirectPolicy);
        QNetworkReply *reply=net->get(req); QEventLoop loop; connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit); QTimer timer; timer.setSingleShot(true); connect(&timer,&QTimer::timeout,reply,&QNetworkReply::abort); timer.start(30000); loop.exec();
        int http=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(); bool good=reply->error()==QNetworkReply::NoError&&http>=200&&http<300; QByteArray body=reply->readAll(); QString err=reply->errorString(); reply->deleteLater(); if(!good)note("Modrinth request failed (HTTP "+QString::number(http)+"): "+err); if(ok)*ok=good; return good?body:QByteArray();
    }
    QString selectedModLoader() const { return modLoaderBox?modLoaderBox->currentText().toLower():QString("forge"); }
    void searchModrinth(){
        const QString query=modrinthSearchEdit->text().trimmed(); const QString mc=selectedMinecraftBaseVersion(); if(query.isEmpty()||mc.isEmpty())return;
        QJsonArray facets; facets.append(QJsonArray{QString("project_type:mod")}); facets.append(QJsonArray{QString("versions:")+mc}); facets.append(QJsonArray{QString("categories:")+selectedModLoader()});
        QUrl url("https://api.modrinth.com/v2/search"); QUrlQuery q; q.addQueryItem("query",query); q.addQueryItem("facets",QString::fromUtf8(QJsonDocument(facets).toJson(QJsonDocument::Compact))); q.addQueryItem("limit","30"); q.addQueryItem("index","relevance"); url.setQuery(q);
        note("Searching Modrinth for "+query+" (Minecraft "+mc+", "+selectedModLoader()+")…"); bool ok=false; QByteArray body=publicJsonRequest(url.toString(QUrl::FullyEncoded),&ok); if(!ok)return;
        QJsonArray hits=QJsonDocument::fromJson(body).object().value("hits").toArray(); modrinthResultsTable->setRowCount(0);
        for(const QJsonValue &v:hits){QJsonObject m=v.toObject();int row=modrinthResultsTable->rowCount();modrinthResultsTable->insertRow(row);QString author=m.value("author").toString();auto *name=new QTableWidgetItem(m.value("title").toString());name->setData(Qt::UserRole,m.value("project_id").toString());modrinthResultsTable->setItem(row,0,name);modrinthResultsTable->setItem(row,1,new QTableWidgetItem(author));modrinthResultsTable->setItem(row,2,new QTableWidgetItem(QString::number(m.value("downloads").toVariant().toLongLong())));modrinthResultsTable->setItem(row,3,new QTableWidgetItem(m.value("project_id").toString()));}
        note("Modrinth returned "+QString::number(hits.size())+" matching projects.");
    }
    bool installModrinthVersion(const QString &versionId,QSet<QString> &installed,QSet<QString> &visiting,int depth){
        if(installed.contains(versionId))return true; if(depth>8||visiting.contains(versionId)){note("Skipping cyclic/deep Modrinth dependency: "+versionId);return false;} visiting.insert(versionId);
        bool ok=false; QByteArray body=publicJsonRequest("https://api.modrinth.com/v2/version/"+versionId,&ok); if(!ok){visiting.remove(versionId);return false;} QJsonObject version=QJsonDocument::fromJson(body).object();
        for(const QJsonValue &dv:version.value("dependencies").toArray()){QJsonObject dep=dv.toObject();if(dep.value("dependency_type").toString()!="required")continue;QString depVersion=dep.value("version_id").toString();QString depProject=dep.value("project_id").toString();
            if(depVersion.isEmpty()&&!depProject.isEmpty()){QUrl u("https://api.modrinth.com/v2/project/"+depProject+"/version");QUrlQuery q;q.addQueryItem("game_versions",QString("[\"")+selectedMinecraftBaseVersion()+"\"]");q.addQueryItem("loaders",QString("[\"")+selectedModLoader()+"\"]");u.setQuery(q);QByteArray db=publicJsonRequest(u.toString(QUrl::FullyEncoded),&ok);if(ok){QJsonArray versions=QJsonDocument::fromJson(db).array();if(!versions.isEmpty())depVersion=versions.first().toObject().value("id").toString();}}
            if(depVersion.isEmpty()){note("Could not find a compatible required dependency for project "+depProject);visiting.remove(versionId);return false;}
            if(!installModrinthVersion(depVersion,installed,visiting,depth+1)){visiting.remove(versionId);return false;}
        }
        QJsonArray files=version.value("files").toArray();QJsonObject chosen;for(const QJsonValue &fv:files){QJsonObject f=fv.toObject();if(f.value("primary").toBool()){chosen=f;break;}}if(chosen.isEmpty()&&!files.isEmpty())chosen=files.first().toObject();
        QString url=chosen.value("url").toString(),filename=QFileInfo(chosen.value("filename").toString()).fileName(),expected=chosen.value("hashes").toObject().value("sha1").toString();
        if(url.isEmpty()||filename.isEmpty()||!filename.endsWith(".jar",Qt::CaseInsensitive)){note("Modrinth version has no usable JAR file: "+versionId);visiting.remove(versionId);return false;}
        QString dest=instancePath()+"/game/mods/"+filename;
        if(QFileInfo::exists(dest))dest=instancePath()+"/game/mods/"+version.value("project_id").toString()+"-"+filename;
        if(QFileInfo::exists(dest))dest=instancePath()+"/game/mods/"+version.value("project_id").toString()+"-"+versionId+"-"+filename;
        note("Downloading Modrinth mod: "+filename);if(!download(url,dest,expected)){visiting.remove(versionId);return false;}
        QFile archive(dest);if(!archive.open(QIODevice::ReadOnly)||archive.read(4)!=QByteArray("PK\x03\x04",4)){archive.close();QFile::remove(dest);note("Rejected invalid mod archive: "+filename);visiting.remove(versionId);return false;}
        note("Installed "+filename+".");installed.insert(versionId);visiting.remove(versionId);return true;
    }
    void installSelectedModrinthMod(){
        const int row=modrinthResultsTable->currentRow();if(row<0){QMessageBox::information(this,"Select mod","Select a Modrinth result first.");return;}QString projectId=modrinthResultsTable->item(row,0)->data(Qt::UserRole).toString();QString mc=selectedMinecraftBaseVersion();if(projectId.isEmpty()||mc.isEmpty())return;
        if(selectedModLoader()=="forge"&&(!versionBox->currentText().contains("forge",Qt::CaseInsensitive)||readVersionMetadata(versionBox->currentText()).value("inheritsFrom").toString().isEmpty())){QMessageBox::information(this,"Forge required","Install Forge for this instance and select its Forge profile before installing Forge mods.");return;}
        QUrl url("https://api.modrinth.com/v2/project/"+projectId+"/version");QUrlQuery q;q.addQueryItem("game_versions",QString("[\"")+mc+"\"]");q.addQueryItem("loaders",QString("[\"")+selectedModLoader()+"\"]");url.setQuery(q);note("Finding compatible "+selectedModLoader()+" release for "+mc+"…");bool ok=false;QByteArray body=publicJsonRequest(url.toString(QUrl::FullyEncoded),&ok);if(!ok)return;QJsonArray versions=QJsonDocument::fromJson(body).array();if(versions.isEmpty()){QMessageBox::warning(this,"No compatible release","Modrinth has no release for this Minecraft version and loader.");return;}
        QString versionId=versions.first().toObject().value("id").toString();QSet<QString> installed,visiting;if(!installModrinthVersion(versionId,installed,visiting,0)){QMessageBox::warning(this,"Mod installation incomplete","The mod or one of its required dependencies could not be downloaded. Check Console.");return;}refreshInstalledMods();QMessageBox::information(this,"Mod installed",QString::number(installed.size())+" mod file(s), including required dependencies, were installed. Restart Minecraft to load them.");
    }
    bool download(const QString &url,const QString &dest,const QString &expected={}){
        if(url.trimmed().isEmpty()){note("Missing download URL for: "+dest);return false;}
        if(QFileInfo::exists(dest) && !expected.isEmpty()){
            QFile old(dest);
            if(old.open(QIODevice::ReadOnly) && sha1(old.readAll())==expected.toLower()){
                if(queueTable){int row=addQueueRow(QFileInfo(dest).fileName(),dest,"Completed");queueTable->item(row,4)->setText("Cached");}
                return true;
            }
        } else if(QFileInfo::exists(dest) && QFileInfo(dest).size()>0) {
            if(queueTable){int row=addQueueRow(QFileInfo(dest).fileName(),dest,"Completed");queueTable->item(row,4)->setText("Cached");}
            return true;
        }
        QDir().mkpath(QFileInfo(dest).absolutePath());
        QSaveFile out(dest);
        if(!out.open(QIODevice::WriteOnly)){note("Cannot create download file: "+dest);return false;}
        const int queueRow=queueTable?addQueueRow(QFileInfo(dest).fileName(),dest,"Downloading"):-1;
        QNetworkRequest req{QUrl(url)};
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::NoLessSafeRedirectPolicy);
        req.setHeader(QNetworkRequest::UserAgentHeader,"OfficialCraft/3.4");
        QNetworkReply *r=net->get(req);
        QCryptographicHash hash(QCryptographicHash::Sha1);
        bool writeOk=true;
        QElapsedTimer transferClock; transferClock.start();
        connect(r,&QNetworkReply::readyRead,this,[&]{
            QByteArray chunk=r->readAll();
            if(!chunk.isEmpty()){hash.addData(chunk);if(out.write(chunk)!=chunk.size())writeOk=false;}
        });
        connect(r,&QNetworkReply::downloadProgress,this,[this,queueRow,&transferClock](qint64 got,qint64 total){
            if(total>0){
                progress->setFormat(QString("Downloading… %1% (%2 / %3 MB)").arg(got*100/total).arg(got/1048576.0,0,'f',1).arg(total/1048576.0,0,'f',1));
                if(queueRow>=0 && queueRow<queueTable->rowCount()){
                    queueTable->item(queueRow,2)->setText(QString::number(qMin<qint64>(100,got*100/total))+"%");
                    const double seconds=qMax<qint64>(1,transferClock.elapsed())/1000.0;
                    const double bps=got/seconds;
                    queueTable->item(queueRow,3)->setText(bps>1048576?QString::number(bps/1048576.0,'f',2)+" MB/s":QString::number(bps/1024.0,'f',1)+" KB/s");
                    const qint64 remaining=bps>0?(total-got)/bps:0;
                    queueTable->item(queueRow,4)->setText(QString::number(remaining)+"s");
                }
            }
            QApplication::processEvents();
        });
        QEventLoop loop;
        connect(r,&QNetworkReply::finished,&loop,&QEventLoop::quit);
        QTimer timer; timer.setSingleShot(true);
        connect(&timer,&QTimer::timeout,r,&QNetworkReply::abort);
        connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit);
        timer.start(180000); loop.exec();
        QByteArray tail=r->readAll();
        if(!tail.isEmpty()){hash.addData(tail);if(out.write(tail)!=tail.size())writeOk=false;}
        const int http=r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const bool good=writeOk && r->error()==QNetworkReply::NoError && http>=200 && http<300;
        const QString error=r->errorString();
        r->deleteLater();
        if(!good){
            out.cancelWriting();
            if(queueRow>=0){queueTable->item(queueRow,1)->setText("Failed");queueTable->item(queueRow,2)->setText("HTTP "+QString::number(http));}
            note("Download failed (HTTP "+QString::number(http)+"): "+url+" — "+error);
            return false;
        }
        if(!expected.isEmpty() && hash.result().toHex()!=expected.toLatin1().toLower()){
            out.cancelWriting();
            if(queueRow>=0){queueTable->item(queueRow,1)->setText("Failed");queueTable->item(queueRow,2)->setText("SHA-1 mismatch");}
            note("SHA-1 mismatch; discarded file: "+dest);
            return false;
        }
        if(!out.commit()){
            if(queueRow>=0){queueTable->item(queueRow,1)->setText("Failed");queueTable->item(queueRow,2)->setText("Commit failed");}
            note("Could not finalize download: "+dest);
            return false;
        }
        if(queueRow>=0){queueTable->item(queueRow,1)->setText("Completed");queueTable->item(queueRow,2)->setText("100%");queueTable->item(queueRow,4)->setText("Done");}
        return true;
    }

    struct DownloadTask { QString url, dest, expected; int row=-1; };
    int addQueueRow(const QString &name,const QString &dest,const QString &state){
        int row=queueTable->rowCount(); queueTable->insertRow(row);
        queueTable->setItem(row,0,new QTableWidgetItem(name));
        queueTable->setItem(row,1,new QTableWidgetItem(state));
        queueTable->setItem(row,2,new QTableWidgetItem(state=="Completed"?"100%":"0%"));
        queueTable->setItem(row,3,new QTableWidgetItem("—"));
        queueTable->setItem(row,4,new QTableWidgetItem("—"));
        queueTable->setItem(row,5,new QTableWidgetItem(dest));
        return row;
    }
    bool downloadBatch(QVector<DownloadTask> tasks){
        if(tasks.isEmpty()) return true;
        QSettings settings(configDir()+"/launcher.ini",QSettings::IniFormat);
        const int workers=qBound(1,settings.value("downloadWorkers",6).toInt(),16);
        queueTotal=tasks.size(); queueFinished=0; bool anyFailed=false;
        if(tabs && queueTable) tabs->setCurrentWidget(queueTable->parentWidget());
        QEventLoop loop; int active=0; int next=0;
        for(auto &t:tasks){
            QString label=QFileInfo(t.dest).fileName();
            if(QFileInfo::exists(t.dest)){
                if(t.expected.isEmpty() && QFileInfo(t.dest).size()>0){t.row=addQueueRow(label,t.dest,"Completed"); ++queueFinished; continue;}
                QFile old(t.dest); if(old.open(QIODevice::ReadOnly) && sha1(old.readAll())==t.expected.toLower()){t.row=addQueueRow(label,t.dest,"Completed");++queueFinished;continue;}
            }
            t.row=addQueueRow(label,t.dest,"Queued");
        }
        auto updateRow=[this](int row,const QString &status,const QString &pct){
            if(row<0||row>=queueTable->rowCount())return;
            queueTable->item(row,1)->setText(status); queueTable->item(row,2)->setText(pct);
        };
        std::function<void()> pump;
        pump=[&,this](){
            while(active<workers && next<tasks.size()){
                DownloadTask task=tasks[next++];
                if(queueTable->item(task.row,1)->text()=="Completed") continue;
                ++active; updateRow(task.row,"Downloading","0%");
                QDir().mkpath(QFileInfo(task.dest).absolutePath());
                auto *out=new QSaveFile(task.dest);
                auto *hash=new QCryptographicHash(QCryptographicHash::Sha1);
                auto *writeOk=new bool(true);
                auto *elapsed=new QElapsedTimer; elapsed->start();
                if(!out->open(QIODevice::WriteOnly)){
                    updateRow(task.row,"Failed","Cannot write"); anyFailed=true; delete out; delete hash; delete writeOk; delete elapsed; --active; ++queueFinished; continue;
                }
                QNetworkRequest req{QUrl(task.url)};
                req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::NoLessSafeRedirectPolicy);
                req.setHeader(QNetworkRequest::UserAgentHeader,"OfficialCraft/3.4");
                QNetworkReply *reply=net->get(req);
                connect(reply,&QNetworkReply::readyRead,this,[reply,out,hash,writeOk](){
                    QByteArray chunk=reply->readAll(); if(!chunk.isEmpty()){hash->addData(chunk);if(out->write(chunk)!=chunk.size())*writeOk=false;}
                });
                connect(reply,&QNetworkReply::downloadProgress,this,[this,row=task.row,elapsed,updateRow](qint64 got,qint64 total){
                    if(total>0) {
                        updateRow(row,"Downloading",QString::number(qMin<qint64>(100,got*100/total))+"%");
                        double seconds=qMax<qint64>(1,elapsed->elapsed())/1000.0;
                        double bps=got/seconds;
                        queueTable->item(row,3)->setText(bps>1048576?QString::number(bps/1048576.0,'f',2)+" MB/s":QString::number(bps/1024.0,'f',1)+" KB/s");
                        qint64 remaining=bps>0?(total-got)/bps:0;
                        queueTable->item(row,4)->setText(remaining>3600?QString::number(remaining/3600)+"h "+QString::number((remaining%3600)/60)+"m":remaining>60?QString::number(remaining/60)+"m "+QString::number(remaining%60)+"s":QString::number(remaining)+"s");
                    }
                });
                connect(reply,&QNetworkReply::finished,this,[&,reply,out,hash,writeOk,elapsed,task,updateRow](){
                    QByteArray tail=reply->readAll(); if(!tail.isEmpty()){hash->addData(tail);if(out->write(tail)!=tail.size())*writeOk=false;}
                    int http=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                    bool good=*writeOk && reply->error()==QNetworkReply::NoError && http>=200 && http<300;
                    QString error=reply->errorString();
                    if(good && !task.expected.isEmpty() && hash->result().toHex()!=task.expected.toLatin1().toLower()) {good=false;error="SHA-1 mismatch";}
                    if(good) good=out->commit(); else out->cancelWriting();
                    if(!good){anyFailed=true;updateRow(task.row,"Failed",error.left(24));note("Download failed: "+task.dest+" — "+error);}
                    else {updateRow(task.row,"Completed","100%"); queueTable->item(task.row,4)->setText("Done");}
                    reply->deleteLater(); delete out; delete hash; delete writeOk; delete elapsed; --active; ++queueFinished;
                    progress->setValue(qMin(100,queueFinished*100/qMax(1,queueTotal)));
                    if(next<tasks.size()) pump();
                    if(next>=tasks.size() && active==0) loop.quit();
                });
            }
            if(next>=tasks.size() && active==0) loop.quit();
        };
        pump();
        if(active>0 || next<tasks.size()) loop.exec();
        return !anyFailed;
    }

    void loadManifest() {
        note("Fetching official Minecraft version manifest…");
        bool ok=false;
        QByteArray b=request("https://piston-meta.mojang.com/mc/game/version_manifest_v2.json",&ok);
        if(!ok){note("Could not load version list. Check internet/DNS and restart.");return;}
        manifest=QJsonDocument::fromJson(b).object();
        const QJsonArray versions=manifest.value("versions").toArray();
        {
            // Prevent the combo's change handler from overwriting each instance's
            // saved version with the first item while the manifest is populated.
            const QSignalBlocker blocker(versionBox);
            versionBox->clear();
            for(const auto &entry:versions) { const QJsonObject o=entry.toObject(); if(o.value("type").toString()=="release") versionBox->addItem(o.value("id").toString()); }
        }
        note(QString("Loaded %1 official release versions.").arg(versionBox->count()));
        if(!selected.isEmpty()) loadInstanceSettings();
    }
    bool installVersion(){
        if(selected.isEmpty()||versionBox->currentText().isEmpty()){QMessageBox::warning(this,"Select version","Choose a Minecraft version first.");return false;}
        installBtn->setEnabled(false); launchBtn->setEnabled(false); progress->setValue(0); saveInstanceSettings(); QString id=versionBox->currentText();
        QJsonObject vo; for(auto x:manifest.value("versions").toArray()){auto o=x.toObject();if(o.value("id").toString()==id){vo=o;break;}}
        if(vo.isEmpty()){note("Version metadata is missing; reload the launcher.");goto done;}
        {bool ok=false; QByteArray meta=request(vo.value("url").toString(),&ok); if(!ok)goto done; QJsonObject v=QJsonDocument::fromJson(meta).object(); QString base=instancePath();
         note("Queueing Minecraft "+id+" client…"); QJsonObject client=v.value("downloads").toObject().value("client").toObject();
         if(client.isEmpty()||!downloadBatch({DownloadTask{client.value("url").toString(),base+"/versions/"+id+"/"+id+".jar",client.value("sha1").toString(),-1}})){note("Client JAR download failed.");goto done;}
         QJsonArray libs=v.value("libraries").toArray(); QVector<DownloadTask> libraryTasks;
         QSet<QString> scheduledLibraryPaths;
         for(auto lv:libs){
             QJsonObject lib=lv.toObject(); QJsonObject downloads=lib.value("downloads").toObject();
             // Regular Java artifacts belong on the classpath and must be downloaded.
             QJsonObject artifact=downloads.value("artifact").toObject();
             QString artifactPath=artifact.value("path").toString();
             if(!artifactPath.isEmpty()){
                 QString dest=base+"/libraries/"+artifactPath;
                 if(!scheduledLibraryPaths.contains(dest)){
                     libraryTasks.push_back({artifact.value("url").toString(),dest,artifact.value("sha1").toString(),-1});
                     scheduledLibraryPaths.insert(dest);
                 }
             }
             // Native classifiers are separate JARs; downloading only `artifact` leaves
             // LWJGL's .so files absent (or stale) even though extraction appears to run.
             QString classifier=lib.value("natives").toObject().value("linux").toString();
             classifier.replace("${arch}",sizeof(void*)==8?"64":"32");
             if(!classifier.isEmpty()){
                 QJsonObject nativeArtifact=downloads.value("classifiers").toObject().value(classifier).toObject();
                 QString nativePath=nativeArtifact.value("path").toString();
                 if(!nativePath.isEmpty()){
                     QString dest=base+"/libraries/"+nativePath;
                     if(!scheduledLibraryPaths.contains(dest)){
                         libraryTasks.push_back({nativeArtifact.value("url").toString(),dest,nativeArtifact.value("sha1").toString(),-1});
                         scheduledLibraryPaths.insert(dest);
                     }
                 } else {
                     note("Native classifier metadata missing for library: "+lib.value("name").toString());
                 }
             }
         }
         note(QString("Queueing %1 library and native files with parallel workers…").arg(libraryTasks.size()));
         if(!downloadBatch(libraryTasks)){note("One or more library/native downloads failed. Retry the installation.");goto done;}
         QJsonObject ai=v.value("assetIndex").toObject(); QString indexId=ai.value("id").toString(); if(!indexId.isEmpty()){QString indexPath=base+"/assets/indexes/"+indexId+".json"; if(!downloadBatch({DownloadTask{ai.value("url").toString(),indexPath,ai.value("sha1").toString(),-1}})){note("Asset index download failed.");goto done;} QFile af(indexPath); af.open(QIODevice::ReadOnly); QJsonObject assets=QJsonDocument::fromJson(af.readAll()).object().value("objects").toObject(); QVector<DownloadTask> assetTasks; for(auto it=assets.begin();it!=assets.end();++it){QString hash=it.value().toObject().value("hash").toString(); if(hash.size()<2)continue; QString rel=hash.left(2)+"/"+hash; QString url="https://resources.download.minecraft.net/"+rel; assetTasks.push_back({url,base+"/assets/objects/"+rel,hash,-1});}
         note(QString("Queueing %1 asset files with parallel workers…").arg(assetTasks.size()));
         if(!downloadBatch(assetTasks)){note("One or more asset downloads failed. Retry the installation.");goto done;} }
         // Rebuild the per-instance native directory from verified Linux classifier JARs.
         // Remove stale .so files first so a previous/bad extraction cannot mask a new one.
         QString nativeDir=base+"/natives";
         QDir nativeRoot(nativeDir);
         if(nativeRoot.exists()) nativeRoot.removeRecursively();
         QDir().mkpath(nativeDir);
         int extractedNativeCount=0;
         for(auto lv:libs){
             QJsonObject lib=lv.toObject(); QJsonObject downloads=lib.value("downloads").toObject();
             QString classifier=lib.value("natives").toObject().value("linux").toString();
             classifier.replace("${arch}",sizeof(void*)==8?"64":"32");
             if(classifier.isEmpty()) continue;
             QJsonObject nativeMeta=downloads.value("classifiers").toObject().value(classifier).toObject();
             QString nativePath=nativeMeta.value("path").toString();
             if(nativePath.isEmpty()) continue;
             QString nativeJar=base+"/libraries/"+nativePath;
             if(!QFileInfo::exists(nativeJar) || QFileInfo(nativeJar).size()==0){
                 note("Missing Linux native archive: "+nativePath); continue;
             }
             QString tempDir=nativeDir+"/.extract-"+QUuid::createUuid().toString(QUuid::WithoutBraces);
             QDir().mkpath(tempDir);
             QProcess unzip; unzip.start("unzip",{"-oq",nativeJar,"-d",tempDir});
             if(!unzip.waitForFinished(60000) || unzip.exitCode()!=0){
                 note("Native extraction failed for "+QFileInfo(nativeJar).fileName()+": "+QString::fromLocal8Bit(unzip.readAllStandardError()));
                 QDir(tempDir).removeRecursively(); continue;
             }
             QDirIterator it(tempDir,QDir::Files,QDirIterator::Subdirectories);
             while(it.hasNext()){
                 QString source=it.next(); QString lower=QFileInfo(source).fileName().toLower();
                 if(!lower.endsWith(".so") && !lower.contains(".so.")) continue;
                 QFile in(source); if(!in.open(QIODevice::ReadOnly)) continue;
                 QByteArray magic=in.read(4); in.close();
                 if(magic!=QByteArray("\x7f" "ELF",4)){note("Ignoring non-ELF native file: "+source);continue;}
                 QString target=nativeDir+"/"+QFileInfo(source).fileName();
                 QFile::remove(target);
                 if(QFile::copy(source,target)){
                     QFile::setPermissions(target,QFile::ReadOwner|QFile::WriteOwner|QFile::ReadGroup|QFile::ReadOther);
                     ++extractedNativeCount;
                 }
             }
             QDir(tempDir).removeRecursively();
         }
         note(QString("Extracted %1 Linux native libraries into %2").arg(extractedNativeCount).arg(nativeDir));
         if(extractedNativeCount==0) note("WARNING: No Linux .so natives were extracted. Minecraft may not launch; check the download queue for failed classifier JARs.");
         QDir().mkpath(base+"/versions/"+id);
         QSaveFile versionMetaFile(base+"/versions/"+id+"/"+id+".json");
         if(!versionMetaFile.open(QIODevice::WriteOnly)){note("Could not save version metadata for "+id);goto done;}
         versionMetaFile.write(QJsonDocument(v).toJson(QJsonDocument::Indented));
         if(!versionMetaFile.commit()){note("Could not commit version metadata for "+id);goto done;}
         QJsonObject cfg=readInstance(); cfg["version"]=id; cfg["versionJson"]=v;
         QJsonArray installed=cfg.value("installedVersions").toArray(); bool already=false;
         for(const auto &entry:installed) if(entry.toString()==id) already=true;
         if(!already) installed.append(id); cfg["installedVersions"]=installed;
         if(!writeInstance(cfg)){note("Could not save instance configuration.");goto done;}
         progress->setValue(100); refreshVersionStatus(); note("Minecraft "+id+" files installed and verified."); }
        done: installBtn->setEnabled(true); launchBtn->setEnabled(true); return progress->value()==100;
    }
    bool launchRuleAllowed(const QJsonArray &rules) const {
        if(rules.isEmpty())return true;bool allowed=false;for(const QJsonValue &v:rules){QJsonObject rule=v.toObject();QJsonObject os=rule.value("os").toObject();if(!os.isEmpty()&&!os.value("name").toString().isEmpty()&&os.value("name").toString()!="linux")continue;if(rule.contains("features"))continue;allowed=rule.value("action").toString()=="allow";}return allowed;
    }
    QStringList metadataArguments(const QJsonArray &items,const QMap<QString,QString> &values) const {
        QStringList out;for(const QJsonValue &item:items){if(item.isString()){out<<item.toString();continue;}QJsonObject obj=item.toObject();if(!launchRuleAllowed(obj.value("rules").toArray()))continue;QJsonValue value=obj.value("value");if(value.isString())out<<value.toString();else if(value.isArray())for(const QJsonValue &v:value.toArray())if(v.isString())out<<v.toString();}
        for(QString &arg:out)for(auto it=values.cbegin();it!=values.cend();++it)arg.replace("${"+it.key()+"}",it.value());return out;
    }
    void launchOffline(){
        if(selected.isEmpty())return;const QString id=versionBox->currentText();const QJsonObject child=readVersionMetadata(id);if(child.isEmpty()){QMessageBox::warning(this,"Missing metadata","Version metadata for "+id+" is missing.");return;}
        const QString parentId=child.value("inheritsFrom").toString();const QJsonObject parent=parentId.isEmpty()?QJsonObject():readVersionMetadata(parentId);QJsonObject effective=parent;for(auto it=child.begin();it!=child.end();++it)if(it.key()!="libraries"&&it.key()!="arguments")effective.insert(it.key(),it.value());
        QJsonObject mergedArguments=parent.value("arguments").toObject();const QJsonObject childArguments=child.value("arguments").toObject();for(const QString &key:{QString("jvm"),QString("game")}){QJsonArray merged=mergedArguments.value(key).toArray();for(const QJsonValue &arg:childArguments.value(key).toArray())merged.append(arg);if(!merged.isEmpty())mergedArguments.insert(key,merged);}if(!mergedArguments.isEmpty())effective.insert("arguments",mergedArguments);
        QJsonArray libs=parent.value("libraries").toArray();QSet<QString> names;for(const QJsonValue &v:libs)names.insert(v.toObject().value("name").toString());for(const QJsonValue &v:child.value("libraries").toArray()){QString n=v.toObject().value("name").toString();if(!names.contains(n)){libs.append(v);names.insert(n);}}
        const QString baseId=child.value("jar").toString(parentId.isEmpty()?id:parentId);const QString jar=instancePath()+"/versions/"+baseId+"/"+baseId+".jar";if(!QFileInfo::exists(jar)||QFileInfo(jar).size()==0){QMessageBox::information(this,"Missing client","Base Minecraft client "+baseId+" is missing. Install it first.");return;}
        QString mainClass=child.value("mainClass").toString();if(mainClass.isEmpty())mainClass=parent.value("mainClass").toString();if(mainClass.isEmpty()){note("Missing mainClass in version metadata.");return;}
        QStringList cp;cp<<jar;for(const QJsonValue &lv:libs){QJsonObject lib=lv.toObject();if(!launchRuleAllowed(lib.value("rules").toArray()))continue;QString rel=lib.value("downloads").toObject().value("artifact").toObject().value("path").toString();if(rel.isEmpty())rel=libraryPathFromCoordinate(lib.value("name").toString());if(!rel.isEmpty()){QString full=instancePath()+"/libraries/"+rel;if(QFileInfo::exists(full))cp<<full;else note("Classpath library missing: "+rel);}}
        QString player=playerEdit->text().trimmed();if(player.isEmpty())player="Player";QString uuid=QUuid::createUuid().toString(QUuid::WithoutBraces).remove('-');QString gameDir=instancePath()+"/game";QDir().mkpath(gameDir);QString assets=instancePath()+"/assets";QString index=effective.value("assetIndex").toObject().value("id").toString();
        QMap<QString,QString> vars{{"auth_player_name",player},{"version_name",id},{"game_directory",gameDir},{"assets_root",assets},{"assets_index_name",index},{"auth_uuid",uuid},{"auth_access_token","0"},{"user_type","legacy"},{"version_type",effective.value("type").toString("release")},{"user_properties","{}"},{"clientid",""},{"auth_xuid",""},{"natives_directory",instancePath()+"/natives"},{"launcher_name","OfficialCraft"},{"launcher_version","3.2"},{"classpath",cp.join(QDir::listSeparator())},{"classpath_separator",QDir::listSeparator()},{"library_directory",instancePath()+"/libraries"},{"resolution_width","854"},{"resolution_height","480"}};
        const QString javaPath=chooseJavaRuntime(effective,id,true); if(javaPath.isEmpty())return;
        javaEdit->setText(javaPath); saveInstanceSettings();
        QStringList args;args<<"-Xmx"+QString::number(qBound(512,ramEdit->text().toInt(),32768))+"M"<<"-Djava.library.path="+instancePath()+"/natives"<<"-Dorg.lwjgl.librarypath="+instancePath()+"/natives"<<"-Dnet.java.games.input.librarypath="+instancePath()+"/natives";
        args<<metadataArguments(effective.value("arguments").toObject().value("jvm").toArray(),vars);args<<"-cp"<<cp.join(QDir::listSeparator())<<mainClass;
        QString legacy=child.value("minecraftArguments").toString();if(legacy.isEmpty())legacy=parent.value("minecraftArguments").toString();
        if(!legacy.isEmpty()){QStringList game=QProcess::splitCommand(legacy);for(QString &arg:game)for(auto it=vars.cbegin();it!=vars.cend();++it)arg.replace("${"+it.key()+"}",it.value());args<<game;}
        else {QJsonArray game=effective.value("arguments").toObject().value("game").toArray();if(!game.isEmpty())args<<metadataArguments(game,vars);else args<<"--username"<<player<<"--version"<<id<<"--gameDir"<<gameDir<<"--assetsDir"<<assets<<"--assetIndex"<<index<<"--uuid"<<uuid<<"--accessToken"<<"0"<<"--userProperties"<<"{}"<<"--userType"<<"legacy";}
        note("Starting offline session ("+(parentId.isEmpty()?QString("Vanilla"):QString("inherits from ")+parentId)+"). Online servers/Realms require Microsoft sign-in.");QProcess *p=new QProcess(this);p->setWorkingDirectory(gameDir);connect(p,&QProcess::readyReadStandardOutput,this,[this,p]{QString o=QString::fromLocal8Bit(p->readAllStandardOutput()).trimmed();if(!o.isEmpty())note(o);});connect(p,&QProcess::readyReadStandardError,this,[this,p]{QString o=QString::fromLocal8Bit(p->readAllStandardError()).trimmed();if(!o.isEmpty())note(o);});connect(p,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this,p](int code,QProcess::ExitStatus){note("Minecraft process exited with code "+QString::number(code));p->deleteLater();});note("Launching with Java: "+javaPath);p->start(javaPath,args);if(!p->waitForStarted(5000))note("Could not start Java: "+p->errorString());
    }
};

int main(int argc,char **argv){
    // Avoid loading the GTK/GNOME platform-theme bridge. Some Ubuntu installs
    // ship an xsettings schema without the antialiasing key expected by the
    // bridge, which can abort Qt applications before the window is created.
    qputenv("QT_QPA_PLATFORMTHEME", QByteArray("qt6ct"));
    QApplication app(argc,argv);
    QApplication::setOrganizationName("OfficialCraft");
    QApplication::setApplicationName("OfficialCraft");
    LauncherWindow w; w.show(); return app.exec();
}
#include "main.moc"
