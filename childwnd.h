#ifndef CHILDWND_H
#define CHILDWND_H

#include <QTextEdit>
#include <QCloseEvent>
#include <QTextCharFormat>

//子窗口类
class ChildWnd : public QTextEdit
{
    Q_OBJECT
public:
    ChildWnd();

    QString m_curDocPath;       //当前文档路径

    void newDoc();              //新建文档
    QString getCurDocName();    //文档路径中提取文件名
    bool loadDoc(const QString& docName);//加载文档内容
    void setCurDoc(const QString& docName);//设置当前文档
    bool saveDoc();
    bool saveAsDoc();
    bool saveDocOpt(QString docName);
    void setFormatOnSelectWord(const QTextCharFormat& fmt);
    void setAlignOfDocument(int alignType);          //设置文本对齐
    void setParaStyle(int pstyle);                  //设置项目风格
protected:
    void closeEvent(QCloseEvent* event);
private:
    bool promptSave();          //尝试保存文件
private slots:
    void docBeModified();       //文档修改时，窗口标题栏加'*'
private:
    bool m_bSaved;              //文档是否保存

};

#endif // CHILDWND_H
