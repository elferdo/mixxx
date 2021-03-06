#include <QStylePainter>
#include <QStyleOption>
#include <QSize>
#include <QApplication>

#include "widget/wstarrating.h"

WStarRating::WStarRating(QString group, QWidget* pParent)
        : WWidget(pParent),
          m_starRating(0,5),
          m_pGroup(group),
          m_focused(false) {
}

void WStarRating::setup(const QDomNode& node, const SkinContext& context) {
    Q_UNUSED(node);
    Q_UNUSED(context);
    setMouseTracking(true);
}

QSize WStarRating::sizeHint() const {
    QStyleOption option;
    option.initFrom(this);
    QSize widgetSize = style()->sizeFromContents(QStyle::CT_PushButton, &option,
                                                 m_starRating.sizeHint(), this);

    m_contentRect.setRect(
        (widgetSize.width() - m_starRating.sizeHint().width()) / 2,
        (widgetSize.height() - m_starRating.sizeHint().height()) / 2,
        m_starRating.sizeHint().width(),
        m_starRating.sizeHint().height()
    );

    return widgetSize;
}

void WStarRating::slotTrackLoaded(TrackPointer track) {
    if (track) {
        m_pCurrentTrack = track;
        connect(track.data(), SIGNAL(changed(Track*)),
                this, SLOT(updateRating(Track*)));
        updateRating();
    }
}

void WStarRating::slotTrackUnloaded(TrackPointer track) {
    Q_UNUSED(track);
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.data(), nullptr, this, nullptr);
    }
    m_pCurrentTrack.clear();
    updateRating();
}

void WStarRating::updateRating() {
    if (m_pCurrentTrack) {
        m_starRating.setStarCount(m_pCurrentTrack->getRating());
    } else {
        m_starRating.setStarCount(0);
    }
    update();
}

void WStarRating::updateRating(Track* /*unused*/) {
    updateRating();
}

void WStarRating::paintEvent(QPaintEvent * /*unused*/) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter painter(this);

    painter.setBrush(option.palette.text());
    painter.drawPrimitive(QStyle::PE_Widget, option);

    m_starRating.paint(&painter, m_contentRect);
}

void WStarRating::mouseMoveEvent(QMouseEvent *event) {
    if (!m_pCurrentTrack) {
        return;
    }

    m_focused = true;
    int star = starAtPosition(event->x());

    if (star != m_starRating.starCount() && star != -1) {
        m_starRating.setStarCount(star);
        update();
    }
}

void WStarRating::leaveEvent(QEvent* /*unused*/) {
    m_focused = false;
    updateRating();
}

// The method uses basic linear algebra to find out which star is under the cursor.
int WStarRating::starAtPosition(int x) {
    // If the mouse is very close to the left edge, set 0 stars.
    if (x < m_starRating.sizeHint().width() * 0.05) {
        return 0;
    }
    int star = (x / (m_starRating.sizeHint().width() / m_starRating.maxStarCount())) + 1;

    if (star <= 0 || star > m_starRating.maxStarCount()) {
        return 0;
    }

    return star;
}

void WStarRating::mouseReleaseEvent(QMouseEvent* /*unused*/) {
    if (!m_pCurrentTrack) {
        return;
    }

    m_pCurrentTrack->setRating(m_starRating.starCount());
}

void WStarRating::fillDebugTooltip(QStringList* debug) {
    WWidget::fillDebugTooltip(debug);

    QString currentRating = "-";
    QString maximumRating = QString::number(m_starRating.maxStarCount());

    if (m_pCurrentTrack) {
        currentRating.setNum(m_pCurrentTrack->getRating());
    }

    *debug << QString("Rating: %1/%2").arg(currentRating, maximumRating);
}
