#include "ui/lightboxdialog.h"
#include <QVBoxLayout>

LightboxDialog::LightboxDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Image"));
    resize(600, 400);
    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setWidget(m_label);
    auto l = new QVBoxLayout(this);
    l->addWidget(m_scroll);
}

void LightboxDialog::setImage(const QPixmap& pm){
    m_label->setPixmap(pm);
    m_scale = 1.0;
    applyScale();
}

void LightboxDialog::wheelEvent(QWheelEvent* e){
    if (e->angleDelta().y()>0) m_scale *= 1.1;
    else m_scale /= 1.1;
    applyScale();
}

void LightboxDialog::applyScale(){
    if (!m_label->pixmap()) return;
    QSize s = m_label->pixmap()->size()*m_scale;
    m_label->setPixmap(m_label->pixmap()->scaled(s, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
