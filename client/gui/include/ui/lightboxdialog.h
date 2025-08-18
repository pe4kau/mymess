#pragma once
#include <QDialog>
#include <QLabel>
#include <QScrollArea>
#include <QWheelEvent>

class LightboxDialog : public QDialog {
    Q_OBJECT
public:
    explicit LightboxDialog(QWidget* parent=nullptr);
    void setImage(const QPixmap& pm);

protected:
    void wheelEvent(QWheelEvent* e) override;

private:
    QLabel* m_label = nullptr;
    QScrollArea* m_scroll = nullptr;
    double m_scale = 1.0;
    void applyScale();
};
