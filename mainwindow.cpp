#include <QFontDatabase>
#include <QMdiSubWindow>
#include <QList>
#include <QAction>
#include <QFileDialog>
#include <QTextCharFormat>
#include <QActionGroup>
#include <QColorDialog>
#include <QtPrintSupport/QPrinter>
#include <QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "childwnd.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initMainWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initMainWindow()
{
    //初始化字号列表项
    QFontDatabase fontdb;
    foreach(int fontsize, fontdb.standardSizes())
    {
        ui->sizeComboBox->addItem(QString::number(fontsize));
    }
    QFont defaultFont;//当前程序默认的字体
    QString sFontSize;
    int defaultFontSize;//当前应用程序默认的字号

    int defaultFontIndex;//当前字号在组合框中的索引号

    defaultFont = QApplication::font();
    defaultFontSize = defaultFont.pointSize();
    sFontSize = QString::number(defaultFontSize);
    defaultFontIndex = ui->sizeComboBox->findText(sFontSize);

    ui->sizeComboBox->setCurrentIndex(defaultFontIndex);

    //设置滚动条
    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    refreshMenus();
    //子窗口有变动的时候，刷新菜单栏
    connect(ui->mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::refreshMenus);

    //点击菜单栏时，更新窗口菜单
    addSubWndListMenu();
    connect(ui->menu_Window, &QMenu::aboutToShow, this, &MainWindow::addSubWndListMenu);

    //创建信号映射器
    m_WndMapper = new QSignalMapper(this);
//    connect(m_WndMapper, SIGNAL(mapped(QWidget*)),
//            this,SLOT(setActiveSubWindow(QWidget*)));
    connect(m_WndMapper, &QSignalMapper::mappedWidget,this,&MainWindow::setActiveSubWindow);

    //对齐操作具有互斥性，一次只能选择一种，需要放在分组里
    QActionGroup *alignGroup = new QActionGroup(this);
    alignGroup->addAction(ui->leftAlignAction);
    alignGroup->addAction(ui->rightAlignAction);
    alignGroup->addAction(ui->centerAction);
    alignGroup->addAction(ui->justifyAction);

}

void MainWindow::docNew()
{
    ChildWnd* childwnd = new ChildWnd;
    ui->mdiArea->addSubWindow(childwnd);
    //复制信号有效时，剪切复制操作是有效的
    connect(childwnd,SIGNAL(copyAvailable(bool)),ui->cutAction, SLOT(setEnabled(bool)));
    connect(childwnd,SIGNAL(copyAvailable(bool)),ui->copyAction, SLOT(setEnabled(bool)));

    childwnd->newDoc();
    childwnd->show();
    formatEnabled();
}

void MainWindow::docOpen()
{
    auto docName = QFileDialog::getOpenFileName(this,"打开文档","",
                                                "文本文件(*.txt);;HTML文件(*.html *htm);;所有文件(*.*)");
    if(!docName.isEmpty())
    {
        QMdiSubWindow* existWnd = findChildWnd(docName);
        if(existWnd)
        {
            ui->mdiArea->setActiveSubWindow(existWnd);
            return;
        }

        //不存在则创建一个新的
        ChildWnd* childwnd = new ChildWnd;
        ui->mdiArea->addSubWindow(childwnd);

        connect(childwnd,SIGNAL(copyAvailable(bool)), ui->cutAction, SLOT(setEnabled(bool)));
        connect(childwnd,SIGNAL(copyAvailable(bool)), ui->copyAction, SLOT(setEnabled(bool)));

        if(childwnd->loadDoc(docName))
        {
            statusBar()->showMessage("文档已打开", 3000);
            childwnd->show();
            formatEnabled();
        }
        else
        {
            childwnd->close();
        }
    }
}

void MainWindow::docSave()
{
    if(activeChildWnd() && activeChildWnd()->saveDoc())
    {
        statusBar()->showMessage("保存成功", 3000);
    }
}

void MainWindow::docSaveAs()
{
    if(activeChildWnd() && activeChildWnd()->saveAsDoc())
    {
        statusBar()->showMessage("另存为成功", 3000);
    }
}

void MainWindow::docUndo()
{
    if(activeChildWnd())
        activeChildWnd()->undo();
}

void MainWindow::docRedo()
{
    if(activeChildWnd())
        activeChildWnd()->redo();
}

void MainWindow::docCut()
{
    if(activeChildWnd())
        activeChildWnd()->cut();
}

void MainWindow::docCopy()
{
    if(activeChildWnd())
        activeChildWnd()->copy();
}

void MainWindow::docPaste()
{
    if(activeChildWnd())
        activeChildWnd()->paste();
}

void MainWindow::docPrint()
{
    QPrinter pter(QPrinter::HighResolution);
    QPrintDialog* ddlg = new QPrintDialog(&pter,this);
    if(activeChildWnd())
    {
        ddlg->setOption(QAbstractPrintDialog::PrintSelection,true);
    }
    ddlg->setWindowTitle("打印文档");
    ChildWnd* wnd = activeChildWnd();//当前活动的子窗口
    if(ddlg->exec() == QDialog::Accepted)
    {
        wnd->print(&pter);
    }

    delete ddlg;
}

void MainWindow::docPreview()
{
    QPrinter pter;
    QPrintPreviewDialog preview(&pter,this, Qt::WindowMinMaxButtonsHint | Qt::WindowContextHelpButtonHint | Qt::WindowCloseButtonHint);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printPreview(QPrinter*)));
    preview.exec();
}

void MainWindow::textBold()
{
    QTextCharFormat fmt;
    if(!ui->boldAction->isCheckable())
        ui->boldAction->setCheckable(true);
    fmt.setFontWeight(ui->boldAction->isChecked() ? QFont::Bold : QFont::Normal);
    if(activeChildWnd())
        activeChildWnd()->setFormatOnSelectWord(fmt);

}

void MainWindow::textItalic()
{
    QTextCharFormat fmt;
    if(!ui->italicAction->isCheckable())
        ui->italicAction->setCheckable(true);
    fmt.setFontItalic(ui->italicAction->isChecked());
    if(activeChildWnd())
        activeChildWnd()->setFormatOnSelectWord(fmt);
}

void MainWindow::textUnderline()
{
    QTextCharFormat fmt;
    if(!ui->underlineAction->isCheckable())
        ui->underlineAction->setCheckable(true);
    fmt.setFontUnderline(ui->underlineAction->isChecked());
    if(activeChildWnd())
        activeChildWnd()->setFormatOnSelectWord(fmt);
}

void MainWindow::textFamily(const QString &family)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(family);
    if(activeChildWnd())
        activeChildWnd()->setFormatOnSelectWord(fmt);
}

void MainWindow::textSize(const QString &size)
{
    qreal pointSize = size.toFloat();
    if(size.toFloat() > 0)
    {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        if(activeChildWnd())
            activeChildWnd()->setFormatOnSelectWord(fmt);
    }
}

void MainWindow::textColor()
{
    if(activeChildWnd())
    {
        auto color = QColorDialog::getColor(activeChildWnd()->textColor(),this);
        if(!color.isValid())
            return;
        QTextCharFormat fmt;
        fmt.setForeground(color);
        activeChildWnd()->setFormatOnSelectWord(fmt);

        QPixmap pix(16,16);
        pix.fill(color);
        ui->colorAction->setIcon(pix);
    }
}

void MainWindow::paraStyle(int nStyle)
{
    if(activeChildWnd())
    {
        activeChildWnd()->setParaStyle(nStyle);
    }
}

void MainWindow::formatEnabled()
{
    ui->boldAction->setEnabled(true);
    ui->italicAction->setEnabled(true);
    ui->underlineAction->setEnabled(true);
    ui->leftAlignAction->setEnabled(true);
    ui->centerAction->setEnabled(true);
    ui->rightAlignAction->setEnabled(true);
    ui->justifyAction->setEnabled(true);
    ui->colorAction->setEnabled(true);
}

ChildWnd *MainWindow::activeChildWnd()
{
    auto activeSubWindow = ui->mdiArea->activeSubWindow();
    if(activeSubWindow)
    {
        return qobject_cast<ChildWnd*>(activeSubWindow->widget());
    }
    else
    {
        return nullptr;
    }
}

QMdiSubWindow *MainWindow::findChildWnd(const QString &docName)
{
    QString strFile = QFileInfo(docName).canonicalFilePath();
    foreach (QMdiSubWindow* subWnd, ui->mdiArea->subWindowList()) {
        ChildWnd* childwnd = qobject_cast<ChildWnd*>(subWnd->widget());
        if(childwnd->m_curDocPath == strFile)
            return subWnd;
    }
    return nullptr;
}

void MainWindow::refreshMenus()
{
    bool hasChild = activeChildWnd() != nullptr;

    ui->saveAction->setEnabled(hasChild);
    ui->saveAsAction->setEnabled(hasChild);
    ui->printAction->setEnabled(hasChild);
    ui->printPreviewAction->setEnabled(hasChild);
    ui->pasteAction->setEnabled(hasChild);
    ui->closeAction->setEnabled(hasChild);
    ui->closeAllAction->setEnabled(hasChild);
    ui->tileAction->setEnabled(hasChild);
    ui->cascadeAction->setEnabled(hasChild);
    ui->nextAction->setEnabled(hasChild);
    ui->previousAction->setEnabled(hasChild);

    //用户是否选中了文本
    bool hasSelect = activeChildWnd() && activeChildWnd()->textCursor().hasSelection();

    ui->cutAction->setEnabled(hasSelect);
    ui->copyAction->setEnabled(hasSelect);
    ui->boldAction->setEnabled(hasSelect);
    ui->italicAction->setEnabled(hasSelect);
    ui->underlineAction->setEnabled(hasSelect);
    ui->leftAlignAction->setEnabled(hasSelect);
    ui->centerAction->setEnabled(hasSelect);
    ui->rightAlignAction->setEnabled(hasSelect);
    ui->justifyAction->setEnabled(hasSelect);
    ui->colorAction->setEnabled(hasSelect);

    if(hasChild)
    {
        //auto window = activeChildWnd();
    }
}

void MainWindow::addSubWndListMenu()
{
    //不做这一步，每次点击菜单栏都会创建一个菜单列表
    ui->menu_Window->clear();
    ui->menu_Window->addAction(ui->closeAction);
    ui->menu_Window->addAction(ui->closeAllAction);
    ui->menu_Window->addSeparator();
    ui->menu_Window->addAction(ui->tileAction);
    ui->menu_Window->addAction(ui->cascadeAction);
    ui->menu_Window->addSeparator();
    ui->menu_Window->addAction(ui->nextAction);
    ui->menu_Window->addAction(ui->previousAction);

    QList<QMdiSubWindow*> wnds = ui->mdiArea->subWindowList();
    if(!wnds.isEmpty())
    {
        ui->menu_Window->addSeparator();
    }

    for(int i = 0; i < wnds.size(); ++i)
    {
        ChildWnd* childwnd = qobject_cast<ChildWnd*>(wnds.at(i)->widget());
        QString menuitem_text = QString("%1 %2").arg(i+1).arg(childwnd->getCurDocName());

        QAction* menuitem_act = ui->menu_Window->addAction(menuitem_text);
        menuitem_act->setCheckable(true);
        menuitem_act->setChecked(childwnd == activeChildWnd());

        //将当前打开的文档对应菜单栏和mapper对应上
        //connect(menuitem_act,SIGNAL(triggered(bool)), m_WndMapper, SLOT(QSignalMapper::map()));
        connect(menuitem_act,&QAction::triggered, m_WndMapper, QOverload<>::of(&QSignalMapper::map));

        m_WndMapper->setMapping(menuitem_act, wnds.at(i));
    }
    formatEnabled();

}


void MainWindow::on_newAction_triggered()
{
    docNew();
}


void MainWindow::on_closeAction_triggered()
{
    ui->mdiArea->closeActiveSubWindow();
}


void MainWindow::on_closeAllAction_triggered()
{
    ui->mdiArea->closeAllSubWindows();
}


void MainWindow::on_tileAction_triggered()
{
    ui->mdiArea->tileSubWindows();
}


void MainWindow::on_cascadeAction_triggered()
{
    ui->mdiArea->cascadeSubWindows();
}


void MainWindow::on_nextAction_triggered()
{
    ui->mdiArea->activateNextSubWindow();
}


void MainWindow::on_previousAction_triggered()
{
    ui->mdiArea->activatePreviousSubWindow();
}

void MainWindow::setActiveSubWindow(QWidget *wnd)
{
    if(wnd)
        ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(wnd));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    ui->mdiArea->closeAllSubWindows();
    if(ui->mdiArea->currentSubWindow())
        event->ignore();
    else
        event->accept();
}


void MainWindow::on_openAction_triggered()
{
    docOpen();
}


void MainWindow::on_saveAction_triggered()
{
    docSave();
}


void MainWindow::on_saveAsAction_triggered()
{
    docSaveAs();
}


void MainWindow::on_undoAction_triggered()
{
    docUndo();
}


void MainWindow::on_redoAction_triggered()
{
    docRedo();
}


void MainWindow::on_cutAction_triggered()
{
    docCut();
}


void MainWindow::on_copyAction_triggered()
{
    docCopy();
}


void MainWindow::on_pasteAction_triggered()
{
    docPaste();
}


void MainWindow::on_boldAction_triggered()
{
    textBold();
}


void MainWindow::on_italicAction_triggered()
{
    textItalic();
}


void MainWindow::on_underlineAction_triggered()
{
    textUnderline();
}


void MainWindow::on_fontComboBox_textActivated(const QString &arg1)
{
    textFamily(arg1);
}


void MainWindow::on_sizeComboBox_textActivated(const QString &arg1)
{
    textSize(arg1);
}


void MainWindow::on_leftAlignAction_triggered()
{
    if(activeChildWnd())
        activeChildWnd()->setAlignOfDocument(1);
}


void MainWindow::on_centerAction_triggered()
{
    if(activeChildWnd())
        activeChildWnd()->setAlignOfDocument(3);
}


void MainWindow::on_rightAlignAction_triggered()
{
    if(activeChildWnd())
        activeChildWnd()->setAlignOfDocument(2);
}


void MainWindow::on_justifyAction_triggered()
{
    if(activeChildWnd())
        activeChildWnd()->setAlignOfDocument(4);
}


void MainWindow::on_colorAction_triggered()
{
    textColor();
}


void MainWindow::on_comboBox_activated(int index)
{
    paraStyle(index);
}


void MainWindow::on_printAction_triggered()
{
    docPrint();
}


void MainWindow::on_printPreviewAction_triggered()
{
    docPreview();
}

void MainWindow::printPreview(QPrinter *printer)
{
    if(activeChildWnd())
        activeChildWnd()->print(printer);
}

