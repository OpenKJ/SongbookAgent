#ifndef DIALOGABOUT_H
#define DIALOGABOUT_H

#include <QDialog>
#include <memory>

namespace Ui {
class DialogAbout;
}

class DialogAbout : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAbout(QWidget *parent = nullptr);
    ~DialogAbout() override;

private:
    std::unique_ptr<Ui::DialogAbout> m_ui;

};

#endif // DIALOGABOUT_H
