#include "childwnd.h"
#include <QFileInfo>
#include <QFile>
#include <QFileDialog>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QString>
#include <QMessageBox>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextListFormat>
#include <QTextBlockFormat>
#include <QTextList>

ChildWnd::ChildWnd()
{
    //子窗口关闭时销毁该类的实例对象
    setAttribute(Qt::WA_DeleteOnClose);

    m_bSaved = false;
}

void ChildWnd::newDoc()
{
    static int wndSeqNum = 1;
    m_curDocPath = QString("WPS 文档 %1").arg(wndSeqNum++);

    //设置窗体标题，文档改动后名称后加"*"标识
    setWindowTitle(m_curDocPath + "[*]" + "- MyWPS");
//    connect(document(),SIGNAL(contentsChanged()),this,SLOT(docBeModified()));
    connect(document(), &QTextDocument::contentsChanged, this, &ChildWnd::docBeModified);
}

QString ChildWnd::getCurDocName()
{
    return QFileInfo(m_curDocPath).fileName();
}

bool ChildWnd::loadDoc(const QString &docName)
{
    if(!docName.isEmpty())
    {
        QFile file(docName);
        if(!file.exists())
            return false;
        if(!file.open(QIODevice::ReadOnly))
            return false;

        //QByteArray可以直接转QString
        //QString转QByteArray
        /*
          QByteArray by1 = str.toLatin1();
          QByteArray by2 = str.toLocal8Bit();
        */
        auto text = file.readAll();
        if(Qt::mightBeRichText(text))
        {
            setHtml(text);
        }
        else
        {
            setPlainText(text);
        }
        setCurDoc(docName);
        connect(document(), &QTextDocument::contentsChanged, this, &ChildWnd::docBeModified);
        return true;
    }

    return false;
}

void ChildWnd::setCurDoc(const QString &docName)
{
    m_curDocPath = QFileInfo(docName).canonicalFilePath();//返回标准名称路径，过滤路径中的. ..
    m_bSaved = true;                        //文档已被保存
    document()->setModified(false);         //文档未改动
    setWindowModified(false);               //窗口不显示改动标识
    setWindowTitle(getCurDocName() + "[*]");//设置子窗口标题
}

bool ChildWnd::saveDoc()
{
    if(m_bSaved)
        return saveDocOpt(m_curDocPath);
    else
        return saveAsDoc();
}

bool ChildWnd::saveAsDoc()
{
    auto docName = QFileDialog::getSaveFileName(this, "另存为", m_curDocPath,
                                 "HTML文件(*.html *.htm);;所有文件(*.*)");
    if(docName.isEmpty())
        return false;
    else
        return saveDocOpt(docName);
}

//保存文档的操作
bool ChildWnd::saveDocOpt(QString docName)
{
    //将所有文件保存为html格式
    if(!(docName.endsWith(".htm",Qt::CaseInsensitive) || docName.endsWith(".html",Qt::CaseInsensitive)))
    {
        docName = docName + ".html";
    }

    QTextDocumentWriter writer(docName);
    bool isSuccess = writer.write(this->document());
    if(isSuccess)
    {
        setCurDoc(docName);
    }
    return isSuccess;

}

void ChildWnd::setFormatOnSelectWord(const QTextCharFormat &fmt)
{
    QTextCursor cursor = textCursor();
    if(!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(fmt);
    mergeCurrentCharFormat(fmt);
}

void ChildWnd::setAlignOfDocument(int alignType)
{
    if(alignType == 1)
        setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if(alignType == 2)
        setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if(alignType == 3)
        setAlignment(Qt::AlignCenter);
    else if(alignType == 4)
        setAlignment(Qt::AlignJustify);
}

void ChildWnd::setParaStyle(int pstyle)
{
    //获取光标信息
    QTextCursor tcursor = textCursor();

    QTextListFormat::Style sname;
    if(pstyle != 0)
    {
        switch (pstyle)
        {
        case 1:
            sname = QTextListFormat::ListDisc;//黑心实心圆
            break;
        case 2:
            sname = QTextListFormat::ListCircle;//空心圆
            break;
        case 3:
            sname = QTextListFormat::ListSquare;//方形
            break;
        case 4:
            sname = QTextListFormat::ListDecimal;//十进制整数
            break;
        case 5:
            sname = QTextListFormat::ListLowerAlpha;//小写字母
            break;
        case 6:
            sname = QTextListFormat::ListUpperAlpha;//大写字母
            break;
        case 7:
            sname = QTextListFormat::ListLowerRoman;//小写罗马字母
            break;
        case 8:
            sname = QTextListFormat::ListUpperRoman;//大写罗马字母
            break;
        default:
            sname = QTextListFormat::ListDisc;//黑心实心圆
            break;
        }

        tcursor.beginEditBlock();
        QTextBlockFormat tBlockFmt = tcursor.blockFormat();//返回当前光标对应的块格式
        QTextListFormat tListFmt;
        if(tcursor.currentList())
        {
            tListFmt = tcursor.currentList()->format();
        }
        else
        {
            tListFmt.setIndent(tBlockFmt.indent() + 1);//设置缩进
            tBlockFmt.setIndent(0);
            tcursor.setBlockFormat(tBlockFmt);
        }

        tListFmt.setStyle(sname);
        tcursor.createList(tListFmt);

        tcursor.endEditBlock();

    }
    else
    {
        QTextBlockFormat tbfmt;
        tbfmt.setObjectIndex(-1);
        tcursor.mergeBlockFormat(tbfmt);
    }
}

void ChildWnd::closeEvent(QCloseEvent *event)
{
    if(promptSave())
        event->accept();
    else
        event->ignore();
}

bool ChildWnd::promptSave()
{
    if(! document()->isModified()) return true;

    //文档进行了修改
    auto res = QMessageBox::warning(this,"系统提示",QString("文档%1已修改,是否保存？").arg(getCurDocName()),
                         QMessageBox::Yes | QMessageBox::Discard | QMessageBox::No);
    if(res == QMessageBox::Yes)
    {
        return saveDoc();
    }
    else if(res == QMessageBox::No)
    {
        return true;
    }
    return true;
}

void ChildWnd::docBeModified()
{
    setWindowModified(document()->isModified());
}
