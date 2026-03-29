#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTableWidget>
#include <QProgressBar>
#include <QTextEdit>
#include <QButtonGroup>
#include <QListWidget>
#include <QStackedWidget>
#include <QMap>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onBrowseClicked();
    void onOrganizeClicked();
    void onUndoClicked();
    void onModeChanged(int id);
    void onCategoryClicked(QListWidgetItem* item);
    void onExpandTable();

private:
    void setupUI();
    void applyStyles();
    void scanFolder(const QString& path);
    void showResultsBreakdown(const QMap<QString,int>& counts, int total);
    void populateTableForCategory(const QString& category);
    void addLogLine(const QString& text, const QString& color = "#888");
    QMap<QString,QString> getExtMap();
    QMap<QString,QString> getCatColors();
    QMap<QString,QString> getCatTextColors();
    QString getCatIcon(const QString& cat);
    void buildFileMaps(const QString& path);
    void rebuildSidebar(const QString& selectCat = "ALL");
    // void populateTableForCategory(const QString& category);

    // Layout
    QWidget*      centralWidget;
    QWidget*      leftPanel;
    QWidget*      rightPanel;

    // Top bar
    QLineEdit*    folderInput;
    QPushButton*  browseBtn;

    // Mode buttons
    QButtonGroup* modeGroup;
    QPushButton*  modeOrganize;
    QPushButton*  modeByDate;
    QPushButton*  modeDryRun;

    // Stats
    QLabel* statTotal;
    QLabel* statCategories;
    QLabel* statMoved;
    QLabel* statSkipped;

    // Category sidebar
    QListWidget*  categoryList;

    // File table
    QTableWidget* fileTable;
    QPushButton*  expandBtn;
    QLabel*       tableTitle;

    // Progress + log
    QProgressBar* progressBar;
    QLabel*       progressLabel;
    QTextEdit*    logBox;

    // Action buttons
    QPushButton*  organizeBtn;
    QPushButton*  undoBtn;

    // State
    QString currentMode = "organize";
    QMap<QString, QStringList>  categoryFiles;
    QMap<QString, QString>      fileSizes;
    QMap<QString, QString>      fileCategories;
};