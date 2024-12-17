#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QSignalMapper>
#include <QMdiSubWindow>
#include <QtPrintSupport/QPrinter>

#include "childwnd.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void initMainWindow();
    void docNew();              //创建新文档
    void docOpen();             //打开文档
    void docSave();             //文档保存
    void docSaveAs();           //文档另存为
    void docUndo();             //文档撤销
    void docRedo();             //文档不撤销
    void docCut();              //文档剪切
    void docCopy();             //文档复制
    void docPaste();            //文档粘贴
    void docPrint();            //文档打印
    void docPreview();          //文档预览

    void textBold();            //文本加粗
    void textItalic();          //文本倾斜
    void textUnderline();       //文本倾斜
    void textFamily(const QString& family);          //设置字体
    void textSize(const QString& size);
    void textColor();

    void paraStyle(int nStyle);          //项目符号
private slots:
    void on_newAction_triggered();
    void refreshMenus();
    void addSubWndListMenu();
    void on_closeAction_triggered();

    void on_closeAllAction_triggered();

    void on_tileAction_triggered();

    void on_cascadeAction_triggered();

    void on_nextAction_triggered();

    void on_previousAction_triggered();

    void setActiveSubWindow(QWidget*);

    void on_openAction_triggered();

    void on_saveAction_triggered();

    void on_saveAsAction_triggered();

    void on_undoAction_triggered();

    void on_redoAction_triggered();

    void on_cutAction_triggered();

    void on_copyAction_triggered();

    void on_pasteAction_triggered();

    void on_boldAction_triggered();

    void on_italicAction_triggered();

    void on_underlineAction_triggered();

    void on_fontComboBox_textActivated(const QString &arg1);

    void on_sizeComboBox_textActivated(const QString &arg1);

    void on_leftAlignAction_triggered();

    void on_centerAction_triggered();

    void on_rightAlignAction_triggered();

    void on_justifyAction_triggered();

    void on_colorAction_triggered();

    void on_comboBox_activated(int index);

    void on_printAction_triggered();

    void on_printPreviewAction_triggered();

    void printPreview(QPrinter* printer);

protected:
    void closeEvent(QCloseEvent* event);

private:
    void formatEnabled();        //所有格式操作都能启用
    ChildWnd *activeChildWnd();     //当前窗口是否是活动窗口
    QMdiSubWindow* findChildWnd(const QString& docName);

private:
    Ui::MainWindow *ui;
    QSignalMapper* m_WndMapper;     //信号映射器
};
#endif // MAINWINDOW_H
