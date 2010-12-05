#ifndef OPENCLOSEARROW_H
#define OPENCLOSEARROW_H

#include <QLabel>


class OpenCloseArrow : public QLabel
{
    Q_OBJECT

public:
    explicit OpenCloseArrow(QWidget *parent = 0, bool open = true);
    ~OpenCloseArrow();

    bool isOpen() const {return openingState;}

private:
    bool openingState;
    void setOpText();

protected:
    void mouseReleaseEvent( QMouseEvent * ev );

signals:
    void openChanged(bool);

public slots:
    void changeOpen(bool t);

};

#endif // OPENCLOSEARROW_H
