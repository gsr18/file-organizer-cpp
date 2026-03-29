#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QDirIterator>
#include <QProcess>
#include <QFrame>
#include <QTimer>
#include <QDialog>
#include <QListWidgetItem>
#include <QSplitter>
#include <QApplication>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUI();
    applyStyles();
    setWindowTitle("File Organizer");
    setMinimumSize(1000, 720);
    resize(1100, 780);
}
MainWindow::~MainWindow() {}

QString MainWindow::getCatIcon(const QString& cat) {
    if (cat == "Images")    return "🖼";
    if (cat == "Videos")    return "🎬";
    if (cat == "Music")     return "🎵";
    if (cat == "Documents") return "📄";
    if (cat == "Archives")  return "📦";
    if (cat == "Programs")  return "⚙";
    if (cat == "Code")      return "💻";
    return "📁";
}

QMap<QString,QString> MainWindow::getExtMap() {
    return {
        {".jpg","Images"},{".jpeg","Images"},{".png","Images"},
        {".gif","Images"},{".bmp","Images"},{".webp","Images"},{".svg","Images"},
        {".mp4","Videos"},{".mkv","Videos"},{".avi","Videos"},{".mov","Videos"},{".wmv","Videos"},
        {".mp3","Music"},{".wav","Music"},{".flac","Music"},{".aac","Music"},
        {".pdf","Documents"},{".doc","Documents"},{".docx","Documents"},
        {".txt","Documents"},{".pptx","Documents"},{".xlsx","Documents"},{".csv","Documents"},
        {".zip","Archives"},{".rar","Archives"},{".7z","Archives"},{".tar","Archives"},
        {".exe","Programs"},{".msi","Programs"},{".dll","Programs"},
        {".cpp","Code"},{".h","Code"},{".py","Code"},{".js","Code"},{".html","Code"},{".css","Code"}
    };
}
QMap<QString,QString> MainWindow::getCatColors() {
    return {
        {"Images","#E6F1FB"},{"Videos","#E1F5EE"},{"Music","#FBEAF0"},
        {"Documents","#EEEDFE"},{"Archives","#FAEEDA"},
        {"Programs","#FAECE7"},{"Code","#EAF3DE"},{"Others","#F1EFE8"}
    };
}
QMap<QString,QString> MainWindow::getCatTextColors() {
    return {
        {"Images","#185FA5"},{"Videos","#0F6E56"},{"Music","#993556"},
        {"Documents","#3C3489"},{"Archives","#854F0B"},
        {"Programs","#993C1D"},{"Code","#3B6D11"},{"Others","#5F5E5A"}
    };
}

void MainWindow::setupUI() {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* root = new QVBoxLayout(centralWidget);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    // ── Top bar
    QWidget* topBar = new QWidget();
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(64);
    QHBoxLayout* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(24,12,24,12);
    topLayout->setSpacing(12);
    QLabel* appIcon = new QLabel("📂");
    appIcon->setStyleSheet("font-size:22px;");
    QLabel* appTitle = new QLabel("File Organizer");
    appTitle->setObjectName("appTitle");
    folderInput = new QLineEdit();
    folderInput->setObjectName("folderInput");
    folderInput->setPlaceholderText("Click Browse or paste a folder path here...");
    folderInput->setMinimumWidth(340);
    browseBtn = new QPushButton("Browse folder");
    browseBtn->setObjectName("browseBtn");
    topLayout->addWidget(appIcon);
    topLayout->addWidget(appTitle);
    topLayout->addSpacing(16);
    topLayout->addWidget(folderInput, 1);
    topLayout->addWidget(browseBtn);
    root->addWidget(topBar);

    // ── Mode bar
    QWidget* modeBar = new QWidget();
    modeBar->setObjectName("modeBar");
    modeBar->setFixedHeight(52);
    QHBoxLayout* modeLayout = new QHBoxLayout(modeBar);
    modeLayout->setContentsMargins(24,8,24,8);
    modeLayout->setSpacing(8);
    modeOrganize = new QPushButton("  Sort by type");
    modeByDate   = new QPushButton("  Sort by date");
    modeDryRun   = new QPushButton("  Preview only");
    for (auto* btn : {modeOrganize, modeByDate, modeDryRun}) {
        btn->setObjectName("modeBtn");
        btn->setCheckable(true);
        btn->setFixedHeight(34);
    }
    modeOrganize->setChecked(true);
    modeGroup = new QButtonGroup(this);
    modeGroup->addButton(modeOrganize, 0);
    modeGroup->addButton(modeByDate,   1);
    modeGroup->addButton(modeDryRun,   2);
    modeGroup->setExclusive(true);
    QLabel* modeLabel = new QLabel("Mode:");
    modeLabel->setObjectName("modeLabel");
    modeLayout->addWidget(modeLabel);
    modeLayout->addWidget(modeOrganize);
    modeLayout->addWidget(modeByDate);
    modeLayout->addWidget(modeDryRun);
    modeLayout->addStretch();
    undoBtn = new QPushButton("↩  Undo last");
    undoBtn->setObjectName("undoBtn");
    undoBtn->setFixedHeight(34);
    organizeBtn = new QPushButton("▶  Organize now");
    organizeBtn->setObjectName("organizeBtn");
    organizeBtn->setFixedHeight(34);
    modeLayout->addWidget(undoBtn);
    modeLayout->addWidget(organizeBtn);
    root->addWidget(modeBar);

    // ── Stats row
    QWidget* statsRow = new QWidget();
    statsRow->setObjectName("statsRow");
    QHBoxLayout* statsLayout = new QHBoxLayout(statsRow);
    statsLayout->setContentsMargins(24,12,24,12);
    statsLayout->setSpacing(12);
    auto makeStatCard = [&](const QString& icon, const QString& label,
                            QLabel*& val, const QString& col) {
        QWidget* card = new QWidget();
        card->setObjectName("statCard");
        QHBoxLayout* cl = new QHBoxLayout(card);
        cl->setContentsMargins(16,12,16,12);
        cl->setSpacing(12);
        QLabel* ic = new QLabel(icon);
        ic->setStyleSheet("font-size:22px;");
        QVBoxLayout* tl = new QVBoxLayout();
        tl->setSpacing(2);
        QLabel* lb = new QLabel(label);
        lb->setObjectName("statLabel");
        val = new QLabel("—");
        val->setObjectName("statValue");
        val->setStyleSheet("color:"+col+";");
        tl->addWidget(lb);
        tl->addWidget(val);
        cl->addWidget(ic);
        cl->addLayout(tl);
        return card;
    };
    statsLayout->addWidget(makeStatCard("📊","Total files", statTotal,      "#1a1a1a"),1);
    statsLayout->addWidget(makeStatCard("🗂","Categories",  statCategories, "#534AB7"),1);
    statsLayout->addWidget(makeStatCard("✅","Moved",       statMoved,      "#1D9E75"),1);
    statsLayout->addWidget(makeStatCard("⏭","Skipped",     statSkipped,    "#BA7517"),1);
    root->addWidget(statsRow);

    QFrame* div = new QFrame();
    div->setFrameShape(QFrame::HLine);
    div->setObjectName("divider");
    root->addWidget(div);

    // ── Main content: sidebar + table
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->setObjectName("splitter");
    splitter->setHandleWidth(1);

    leftPanel = new QWidget();
    leftPanel->setObjectName("leftPanel");
    leftPanel->setMinimumWidth(200);
    leftPanel->setMaximumWidth(260);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(16,16,16,16);
    leftLayout->setSpacing(8);
    QLabel* catTitle = new QLabel("Categories");
    catTitle->setObjectName("panelTitle");
    leftLayout->addWidget(catTitle);
    categoryList = new QListWidget();
    categoryList->setObjectName("categoryList");
    categoryList->setFrameShape(QFrame::NoFrame);
    leftLayout->addWidget(categoryList);
    QLabel* catHint = new QLabel("Click a category to filter files");
    catHint->setObjectName("hintLabel");
    catHint->setWordWrap(true);
    leftLayout->addWidget(catHint);
    splitter->addWidget(leftPanel);

    rightPanel = new QWidget();
    rightPanel->setObjectName("rightPanel");
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(16,16,16,16);
    rightLayout->setSpacing(10);
    QHBoxLayout* tableHeader = new QHBoxLayout();
    tableTitle = new QLabel("All files");
    tableTitle->setObjectName("panelTitle");
    expandBtn = new QPushButton("⛶  Full screen");
    expandBtn->setObjectName("expandBtn");
    expandBtn->setFixedHeight(28);
    tableHeader->addWidget(tableTitle);
    tableHeader->addStretch();
    tableHeader->addWidget(expandBtn);
    rightLayout->addLayout(tableHeader);

    fileTable = new QTableWidget();
    fileTable->setObjectName("fileTable");
    fileTable->setColumnCount(3);
    fileTable->setHorizontalHeaderLabels({"  File name","Category","Size"});
    fileTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    fileTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    fileTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    fileTable->setColumnWidth(1,120);
    fileTable->setColumnWidth(2,90);
    fileTable->verticalHeader()->setVisible(false);
    fileTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    fileTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    fileTable->setAlternatingRowColors(true);
    fileTable->setShowGrid(false);
    rightLayout->addWidget(fileTable);
    splitter->addWidget(rightPanel);
    splitter->setSizes({220,700});
    root->addWidget(splitter,1);

    // ── Bottom
    QWidget* bottomPanel = new QWidget();
    bottomPanel->setObjectName("bottomPanel");
    QVBoxLayout* bottomLayout = new QVBoxLayout(bottomPanel);
    bottomLayout->setContentsMargins(24,12,24,16);
    bottomLayout->setSpacing(8);
    QHBoxLayout* progRow = new QHBoxLayout();
    progressLabel = new QLabel("Ready");
    progressLabel->setObjectName("progressLabel");
    progressBar = new QProgressBar();
    progressBar->setObjectName("progressBar");
    progressBar->setValue(0);
    progressBar->setTextVisible(false);
    progressBar->setFixedHeight(6);
    progRow->addWidget(progressLabel);
    progRow->addWidget(progressBar,1);
    bottomLayout->addLayout(progRow);
    logBox = new QTextEdit();
    logBox->setObjectName("logBox");
    logBox->setReadOnly(true);
    logBox->setFixedHeight(100);
    logBox->setPlaceholderText("Select a folder above to get started...");
    bottomLayout->addWidget(logBox);
    root->addWidget(bottomPanel);

    connect(browseBtn,    &QPushButton::clicked,     this, &MainWindow::onBrowseClicked);
    connect(organizeBtn,  &QPushButton::clicked,     this, &MainWindow::onOrganizeClicked);
    connect(undoBtn,      &QPushButton::clicked,     this, &MainWindow::onUndoClicked);
    connect(expandBtn,    &QPushButton::clicked,     this, &MainWindow::onExpandTable);
    connect(modeGroup,    &QButtonGroup::idClicked,  this, &MainWindow::onModeChanged);
    connect(categoryList, &QListWidget::itemClicked, this, &MainWindow::onCategoryClicked);
}

void MainWindow::applyStyles() {
    setStyleSheet(R"(
        QMainWindow,QWidget{background:#f5f4f0;font-family:'Segoe UI',sans-serif;font-size:13px;color:#1a1a1a;}
        QWidget#topBar{background:#ffffff;border-bottom:1px solid #e8e7e2;}
        QLabel#appTitle{font-size:16px;font-weight:500;color:#1a1a1a;}
        QLineEdit#folderInput{background:#f5f4f0;border:1px solid #d8d7d2;border-radius:8px;padding:8px 14px;font-size:13px;color:#3d3d3a;}
        QLineEdit#folderInput:focus{border-color:#7F77DD;background:#fff;}
        QPushButton#browseBtn{background:#534AB7;color:#fff;border:none;border-radius:8px;padding:8px 18px;font-size:13px;font-weight:500;}
        QPushButton#browseBtn:hover{background:#4840a0;}
        QWidget#modeBar{background:#fafaf7;border-bottom:1px solid #e8e7e2;}
        QLabel#modeLabel{font-size:12px;color:#888780;font-weight:500;margin-right:4px;}
        QPushButton#modeBtn{background:#fff;border:1px solid #d8d7d2;border-radius:8px;padding:6px 16px;font-size:12px;color:#5f5e5a;}
        QPushButton#modeBtn:checked{background:#EEEDFE;border-color:#AFA9EC;color:#3C3489;font-weight:500;}
        QPushButton#modeBtn:hover{background:#f0efe8;}
        QPushButton#organizeBtn{background:#534AB7;color:#fff;border:none;border-radius:8px;padding:6px 20px;font-size:13px;font-weight:500;}
        QPushButton#organizeBtn:hover{background:#4840a0;}
        QPushButton#undoBtn{background:#fff;border:1px solid #E24B4A;border-radius:8px;padding:6px 16px;color:#A32D2D;font-size:13px;}
        QPushButton#undoBtn:hover{background:#FCEBEB;}
        QWidget#statsRow{background:#ffffff;}
        QWidget#statCard{background:#f5f4f0;border-radius:12px;border:1px solid #e8e7e2;}
        QLabel#statLabel{font-size:11px;color:#888780;}
        QLabel#statValue{font-size:24px;font-weight:500;}
        QFrame#divider{color:#e8e7e2;}
        QWidget#leftPanel{background:#ffffff;border-right:1px solid #e8e7e2;}
        QLabel#panelTitle{font-size:13px;font-weight:500;color:#1a1a1a;margin-bottom:4px;}
        QLabel#hintLabel{font-size:11px;color:#aaa89f;}
        QListWidget#categoryList{background:transparent;border:none;font-size:13px;outline:0;}
        QListWidget#categoryList::item{padding:10px 12px;border-radius:8px;margin-bottom:3px;color:#3d3d3a;}
        QListWidget#categoryList::item:hover{background:#f5f4f0;}
        QListWidget#categoryList::item:selected{background:#EEEDFE;color:#3C3489;}
        QWidget#rightPanel{background:#ffffff;}
        QPushButton#expandBtn{background:transparent;border:1px solid #d8d7d2;border-radius:6px;padding:4px 10px;font-size:11px;color:#888780;}
        QPushButton#expandBtn:hover{background:#f5f4f0;}
        QTableWidget#fileTable{background:#ffffff;border:none;font-size:13px;outline:0;}
        QTableWidget#fileTable::item{padding:10px 12px;border-bottom:1px solid #f0efe8;}
        QTableWidget#fileTable::item:alternate{background:#fafaf8;}
        QTableWidget#fileTable::item:selected{background:#EEEDFE;color:#3C3489;}
        QHeaderView::section{background:#ffffff;border:none;border-bottom:2px solid #e8e7e2;padding:8px 12px;font-size:11px;font-weight:500;color:#888780;}
        QWidget#bottomPanel{background:#ffffff;border-top:1px solid #e8e7e2;}
        QLabel#progressLabel{font-size:12px;color:#888780;min-width:80px;}
        QProgressBar#progressBar{background:#e8e7e2;border-radius:3px;border:none;}
        QProgressBar#progressBar::chunk{background:#534AB7;border-radius:3px;}
        QTextEdit#logBox{background:#18181b;border-radius:10px;border:none;font-family:'Consolas',monospace;font-size:12px;color:#a1a1aa;padding:10px;}
        QSplitter#splitter::handle{background:#e8e7e2;}
    )");
}

void MainWindow::onModeChanged(int id) {
    QStringList modes = {"organize","bydate","dryrun"};
    if (id >= 0 && id < modes.size()) currentMode = modes[id];

    // Restore table to normal 3-column view when leaving dryrun
    if (currentMode != "dryrun") {
        fileTable->setColumnCount(3);
        fileTable->setHorizontalHeaderLabels({"  File name","Category","Size"});
        fileTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        fileTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
        fileTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
        fileTable->setColumnWidth(1,120);
        fileTable->setColumnWidth(2,90);
        QString folder = folderInput->text().trimmed();
        if (!folder.isEmpty()) populateTableForCategory("ALL");
    }
}

void MainWindow::onBrowseClicked() {
    QString dir = QFileDialog::getExistingDirectory(
        this,"Select folder to organize",QDir::homePath(),QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty()) { folderInput->setText(dir); scanFolder(dir); }
}

// ── Core: build category + file maps from a folder ───────────────────────────
void MainWindow::buildFileMaps(const QString& path) {
    categoryFiles.clear();
    fileCategories.clear();
    fileSizes.clear();

    auto extMap    = getExtMap();
    QStringList skip = {"rules.txt","organizer_undo.log"};

    QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        it.next();
        QFileInfo info = it.fileInfo();
        if (skip.contains(info.fileName())) continue;
        QString cat = extMap.value("."+info.suffix().toLower(),"Others");
        categoryFiles[cat].append(info.fileName());
        fileCategories[info.fileName()] = cat;
        double kb = info.size()/1024.0;
        fileSizes[info.fileName()] = kb < 1024
                                         ? QString::number(kb,'f',0)+" KB"
                                         : QString::number(kb/1024.0,'f',1)+" MB";
    }
}

// ── Rebuild sidebar from current maps ────────────────────────────────────────
void MainWindow::rebuildSidebar(const QString& selectCat) {
    categoryList->clear();
    int total = fileCategories.size();

    QListWidgetItem* allItem = new QListWidgetItem("  📂  All files  ("+QString::number(total)+")");
    allItem->setData(Qt::UserRole,"ALL");
    allItem->setFont(QFont("Segoe UI",11,QFont::Medium));
    categoryList->addItem(allItem);

    for (auto it = categoryFiles.begin(); it != categoryFiles.end(); ++it) {
        QString icon = getCatIcon(it.key());
        QListWidgetItem* item = new QListWidgetItem(
            "  "+icon+"  "+it.key()+"  ("+QString::number(it.value().size())+")");
        item->setData(Qt::UserRole, it.key());
        categoryList->addItem(item);
    }

    // Select the right item
    for (int i = 0; i < categoryList->count(); i++) {
        if (categoryList->item(i)->data(Qt::UserRole).toString() == selectCat) {
            categoryList->setCurrentRow(i);
            break;
        }
    }
    if (categoryList->currentRow() < 0) categoryList->setCurrentRow(0);
}

// ── Populate file table from current maps ────────────────────────────────────
void MainWindow::populateTableForCategory(const QString& category) {
    auto catColors     = getCatColors();
    auto catTextColors = getCatTextColors();
    fileTable->setRowCount(0);

    QStringList files = (category == "ALL")
                            ? fileCategories.keys()
                            : categoryFiles.value(category);

    QString icon = (category=="ALL") ? "📂" : getCatIcon(category);
    tableTitle->setText(icon+"  "+(category=="ALL" ? "All files" : category)
                        +"  ·  "+QString::number(files.size())+" files");

    for (const QString& fname : files) {
        QString cat = (category=="ALL") ? fileCategories.value(fname,"Others") : category;
        int row = fileTable->rowCount();
        fileTable->insertRow(row);
        fileTable->setRowHeight(row,38);
        fileTable->setItem(row,0,new QTableWidgetItem("  "+fname));

        QTableWidgetItem* catItem = new QTableWidgetItem(cat);
        catItem->setBackground(QColor(catColors.value(cat,"#F1EFE8")));
        catItem->setForeground(QColor(catTextColors.value(cat,"#5F5E5A")));
        catItem->setTextAlignment(Qt::AlignCenter);
        fileTable->setItem(row,1,catItem);

        QTableWidgetItem* szItem = new QTableWidgetItem(fileSizes.value(fname,"—"));
        szItem->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        fileTable->setItem(row,2,szItem);
    }
}

// ── Scan folder ───────────────────────────────────────────────────────────────
void MainWindow::scanFolder(const QString& path) {
    progressBar->setValue(0);
    logBox->clear();
    progressLabel->setText("Scanning...");

    buildFileMaps(path);

    int total = fileCategories.size();
    statTotal->setText(QString::number(total));
    statCategories->setText(QString::number(categoryFiles.size()));
    statMoved->setText("—");
    statSkipped->setText("—");

    rebuildSidebar("ALL");
    populateTableForCategory("ALL");

    progressLabel->setText("Ready");
    addLogLine("📂  Folder scanned — "+QString::number(total)+" files in "
                   +QString::number(categoryFiles.size())+" categories", "#1D9E75");
    addLogLine("Select a mode and click  ▶ Organize now  to begin.", "#a1a1aa");
}

void MainWindow::onCategoryClicked(QListWidgetItem* item) {
    populateTableForCategory(item->data(Qt::UserRole).toString());
}

// ── Full screen dialog ────────────────────────────────────────────────────────
void MainWindow::onExpandTable() {
    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle(tableTitle->text());
    dlg->resize(1200, 760);
    dlg->setStyleSheet(styleSheet());

    QVBoxLayout* l = new QVBoxLayout(dlg);
    l->setContentsMargins(24, 20, 24, 16);
    l->setSpacing(12);

    // Header row
    QHBoxLayout* headerRow = new QHBoxLayout();
    QLabel* heading = new QLabel(tableTitle->text());
    heading->setStyleSheet("font-size:15px;font-weight:500;color:#1a1a1a;");
    QLabel* hint = new QLabel(currentMode == "dryrun"
                                  ? "👁  Preview mode — no files have been moved"
                                  : "Showing files in selected category");
    hint->setStyleSheet("font-size:12px;color:#888780;");
    headerRow->addWidget(heading);
    headerRow->addStretch();
    headerRow->addWidget(hint);
    l->addLayout(headerRow);

    // Separator
    QFrame* sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color:#e8e7e2;");
    l->addWidget(sep);

    // Build table matching current column count
    int cols = fileTable->columnCount();
    QTableWidget* bigTable = new QTableWidget(dlg);
    bigTable->setObjectName("fileTable");
    bigTable->setColumnCount(cols);
    bigTable->verticalHeader()->setVisible(false);
    bigTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bigTable->setAlternatingRowColors(true);
    bigTable->setShowGrid(false);

    // Set headers + column widths based on mode
    if (currentMode == "dryrun") {
        bigTable->setHorizontalHeaderLabels({"  File name", "Current location", "→ Will move to", "Size"});
        bigTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        bigTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
        bigTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
        bigTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
        bigTable->setColumnWidth(1, 160);
        bigTable->setColumnWidth(2, 180);
        bigTable->setColumnWidth(3, 90);
    } else {
        bigTable->setHorizontalHeaderLabels({"  File name", "Category", "Size"});
        bigTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        bigTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
        bigTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
        bigTable->setColumnWidth(1, 150);
        bigTable->setColumnWidth(2, 100);
    }

    // Copy rows from fileTable
    for (int r = 0; r < fileTable->rowCount(); r++) {
        bigTable->insertRow(r);
        bigTable->setRowHeight(r, 42);
        for (int c = 0; c < cols; c++) {
            QTableWidgetItem* src = fileTable->item(r, c);
            if (!src) continue;
            QTableWidgetItem* copy = new QTableWidgetItem(src->text());
            copy->setBackground(src->background());
            copy->setForeground(src->foreground());
            copy->setTextAlignment(src->textAlignment());
            bigTable->setItem(r, c, copy);
        }
    }

    l->addWidget(bigTable, 1);

    // Footer
    QFrame* sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("color:#e8e7e2;");
    l->addWidget(sep2);

    QHBoxLayout* footer = new QHBoxLayout();
    QLabel* countLabel = new QLabel(QString::number(fileTable->rowCount()) + " files");
    countLabel->setStyleSheet("font-size:12px;color:#888780;");
    QPushButton* closeBtn = new QPushButton("Close");
    closeBtn->setFixedWidth(100);
    closeBtn->setStyleSheet(
        "background:#534AB7;color:#fff;border:none;"
        "border-radius:8px;padding:8px 16px;font-size:13px;");
    footer->addWidget(countLabel);
    footer->addStretch();
    footer->addWidget(closeBtn);
    l->addLayout(footer);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    dlg->exec();
}

void MainWindow::addLogLine(const QString& text, const QString& color) {
    logBox->append(QString("<span style='color:%1;font-family:Consolas,monospace'>%2</span>")
                       .arg(color,text.toHtmlEscaped()));
}

// ── Organize ──────────────────────────────────────────────────────────────────
void MainWindow::onOrganizeClicked() {
    QString folder = folderInput->text().trimmed();
    if (folder.isEmpty()) {
        addLogLine("⚠  Please select a folder first.","#E24B4A"); return;
    }

    // Snapshot counts BEFORE moving
    int totalFiles = fileCategories.size();
    QMap<QString,int> snapshotCounts;
    for (auto it = categoryFiles.begin(); it != categoryFiles.end(); ++it)
        snapshotCounts[it.key()] = it.value().size();

    if (currentMode == "dryrun") {
        logBox->clear();
        progressBar->setValue(0);
        progressLabel->setText("Preview mode");

        // Add a 4th column for destination
        fileTable->setColumnCount(4);
        fileTable->setHorizontalHeaderLabels({"  File name", "Current location", "→ Will move to", "Size"});
        fileTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        fileTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
        fileTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
        fileTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
        fileTable->setColumnWidth(1, 120);
        fileTable->setColumnWidth(2, 130);
        fileTable->setColumnWidth(3, 80);
        fileTable->setRowCount(0);

        auto catColors     = getCatColors();
        auto catTextColors = getCatTextColors();
        int row = 0;

        for (auto it = categoryFiles.begin(); it != categoryFiles.end(); ++it) {
            for (const QString& fname : it.value()) {
                fileTable->insertRow(row);
                fileTable->setRowHeight(row, 38);

                fileTable->setItem(row, 0, new QTableWidgetItem("  " + fname));

                // Current location = root folder (not yet moved)
                QTableWidgetItem* curItem = new QTableWidgetItem("  Downloads/");
                curItem->setForeground(QColor("#888780"));
                fileTable->setItem(row, 1, curItem);

                // Destination
                QTableWidgetItem* destItem = new QTableWidgetItem("→  " + it.key() + "/");
                destItem->setBackground(QColor(catColors.value(it.key(), "#F1EFE8")));
                destItem->setForeground(QColor(catTextColors.value(it.key(), "#5F5E5A")));
                destItem->setTextAlignment(Qt::AlignCenter);
                fileTable->setItem(row, 2, destItem);

                QTableWidgetItem* szItem = new QTableWidgetItem(fileSizes.value(fname, "—"));
                szItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                fileTable->setItem(row, 3, szItem);
                row++;
            }
        }

        tableTitle->setText("👁  Preview  ·  " + QString::number(row) + " files will be organized");

        // Animate progress to show it's "planning"
        for (int i = 0; i <= 100; i += 5) {
            progressBar->setValue(i);
            QApplication::processEvents();
        }
        progressLabel->setText("Preview ready");

        // Log summary
        addLogLine("👁  Preview — no files have been moved yet", "#BA7517");
        addLogLine("──────────────────────────────", "#333");
        for (auto it = categoryFiles.begin(); it != categoryFiles.end(); ++it) {
            addLogLine("  → " + it.key() + "/   " + QString::number(it.value().size()) + " files",
                       catTextColors.value(it.key(), "#888"));
        }
        addLogLine("──────────────────────────────", "#333");
        addLogLine("Switch to 'Sort by type' and click Organize now to apply.", "#555");
        return;
    }

    logBox->clear();
    progressLabel->setText("Organizing...");
    addLogLine("⏳  Organizing "+QString::number(totalFiles)+" files...","#BA7517");
    progressBar->setValue(20);

    QString exePath = R"(C:\Users\gsr58\Desktop\Semester-3\Visual Studio\FileOrganizer\x64\Debug\FileOrganizer.exe)";
    QProcess* proc = new QProcess(this);
    proc->setWorkingDirectory(folder);

    connect(proc, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc, folder, totalFiles, snapshotCounts](int, QProcess::ExitStatus) {
                progressBar->setValue(100);
                progressLabel->setText("Done!");
                statMoved->setText(QString::number(totalFiles));
                statSkipped->setText("0");

                // After moving, scan the subfolders that were created
                // so the sidebar shows the actual organized categories
                auto catColors     = getCatColors();
                auto catTextColors = getCatTextColors();

                // Rebuild maps from the now-organized subfolders
                categoryFiles.clear();
                fileCategories.clear();
                fileSizes.clear();

                for (const QString& cat : snapshotCounts.keys()) {
                    QString subFolder = folder + "/" + cat;
                    if (!QDir(subFolder).exists()) continue;
                    QDirIterator it(subFolder, QDir::Files | QDir::NoDotAndDotDot);
                    while (it.hasNext()) {
                        it.next();
                        QFileInfo info = it.fileInfo();
                        categoryFiles[cat].append(info.fileName());
                        fileCategories[info.fileName()] = cat;
                        double kb = info.size()/1024.0;
                        fileSizes[info.fileName()] = kb < 1024
                                                         ? QString::number(kb,'f',0)+" KB"
                                                         : QString::number(kb/1024.0,'f',1)+" MB";
                    }
                }

                statTotal->setText(QString::number(fileCategories.size()));
                statCategories->setText(QString::number(categoryFiles.size()));

                rebuildSidebar("ALL");
                populateTableForCategory("ALL");

                // Clean log
                logBox->clear();
                addLogLine("✅  Organization complete!","#1D9E75");
                addLogLine("──────────────────────────────","#333");
                for (auto it = snapshotCounts.begin(); it != snapshotCounts.end(); ++it) {
                    int pct = totalFiles > 0 ? (it.value()*100/totalFiles):0;
                    QString bar = QString("█").repeated(qMin(pct/4,25));
                    addLogLine(QString("%1  %2  %3 files")
                                   .arg(it.key(),-12).arg(bar,-26).arg(it.value()),
                               catTextColors.value(it.key(),"#888"));
                }
                addLogLine("──────────────────────────────","#333");
                addLogLine(QString("Moved: %1  ·  Skipped: 0").arg(totalFiles),"#a1a1aa");
                addLogLine("Click  ↩ Undo last  to reverse all changes.","#555");

                proc->deleteLater();
            });

    proc->start(exePath,{});
    proc->waitForStarted();
    proc->write(QString(currentMode+"\n"+folder+"\nyes\n").toLocal8Bit());
    proc->closeWriteChannel();
    progressBar->setValue(60);
}

// ── Undo ──────────────────────────────────────────────────────────────────────
void MainWindow::onUndoClicked() {
    QString folder = folderInput->text().trimmed();
    if (folder.isEmpty()) {
        addLogLine("⚠  Please select a folder first.","#E24B4A"); return;
    }
    logBox->clear();
    progressLabel->setText("Undoing...");
    addLogLine("↩  Restoring all files...","#BA7517");
    progressBar->setValue(30);

    QString exePath = R"(C:\Users\gsr58\Desktop\Semester-3\Visual Studio\FileOrganizer\x64\Debug\FileOrganizer.exe)";
    QProcess* proc = new QProcess(this);

    connect(proc, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this,proc,folder](int, QProcess::ExitStatus) {
                progressBar->setValue(100);
                progressLabel->setText("Restored!");
                addLogLine("✅  All files restored to original locations.","#1D9E75");
                proc->deleteLater();
                QTimer::singleShot(500, this, [this,folder](){ scanFolder(folder); });
            });

    proc->start(exePath,{});
    proc->waitForStarted();
    proc->write(QString("undo\n"+folder+"\n").toLocal8Bit());
    proc->closeWriteChannel();
    progressBar->setValue(60);
}