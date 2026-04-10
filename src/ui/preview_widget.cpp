#include "ui/preview_widget.h"

#include <QAudioOutput>
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSlider>
#include <QStackedWidget>
#include <QStyle>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QVideoWidget>

namespace ui {

namespace {

QString formatTime(qint64 ms) {
  if (ms < 0) {
    ms = 0;
  }
  const qint64 totalSeconds = ms / 1000;
  const int hours = static_cast<int>(totalSeconds / 3600);
  const int minutes = static_cast<int>((totalSeconds % 3600) / 60);
  const int seconds = static_cast<int>(totalSeconds % 60);
  if (hours > 0) {
    return QStringLiteral("%1:%2:%3")
        .arg(hours)
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
  }
  return QStringLiteral("%1:%2").arg(minutes).arg(seconds, 2, 10,
                                                  QLatin1Char('0'));
}

QIcon makeButtonIcon(bool play, const QColor &color) {
  const int sz = 20;
  QPixmap pm(sz, sz);
  pm.fill(Qt::transparent);
  QPainter p(&pm);
  p.setRenderHint(QPainter::Antialiasing);
  p.setPen(Qt::NoPen);
  p.setBrush(color);
  if (play) {
    QPolygonF tri;
    tri << QPointF(4, 2) << QPointF(18, 10) << QPointF(4, 18);
    p.drawPolygon(tri);
  } else {
    p.drawRoundedRect(QRectF(3, 2, 5, 16), 1.5, 1.5);
    p.drawRoundedRect(QRectF(12, 2, 5, 16), 1.5, 1.5);
  }
  return QIcon(pm);
}

QPixmap makeOverlayPixmap(bool play) {
  const int sz = 96;
  QPixmap pm(sz, sz);
  pm.fill(Qt::transparent);
  QPainter p(&pm);
  p.setRenderHint(QPainter::Antialiasing);
  p.setPen(Qt::NoPen);
  p.setBrush(QColor(0, 0, 0, 140));
  p.drawEllipse(0, 0, sz, sz);
  p.setBrush(Qt::white);
  if (play) {
    QPolygonF tri;
    tri << QPointF(33, 24) << QPointF(70, 48) << QPointF(33, 72);
    p.drawPolygon(tri);
  } else {
    p.drawRoundedRect(QRectF(26, 26, 16, 44), 3, 3);
    p.drawRoundedRect(QRectF(54, 26, 16, 44), 3, 3);
  }
  return pm;
}

} // namespace

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent), player_(new QMediaPlayer(this)),
      audio_(new QAudioOutput(this)), stack_(new QStackedWidget(this)),
      placeholderLabel_(nullptr), video_(new QVideoWidget(this)),
      overlayLabel_(nullptr), overlayEffect_(nullptr), overlayAnim_(nullptr),
      overlayFadeTimer_(nullptr), dotTimer_(nullptr),
      playPauseButton_(new QPushButton(this)),
      seekSlider_(new QSlider(Qt::Horizontal, this)),
      timeLabel_(new QLabel(QStringLiteral("0:00 / 0:00"), this)) {
  player_->setVideoOutput(video_);
  player_->setAudioOutput(audio_);

  const QColor iconColor = palette().color(QPalette::ButtonText);
  playIcon_ = makeButtonIcon(true, iconColor);
  pauseIcon_ = makeButtonIcon(false, iconColor);

  placeholder_ = new QWidget(this);
  placeholder_->setAutoFillBackground(true);
  QPalette pal = placeholder_->palette();
  pal.setColor(QPalette::Window, Qt::black);
  placeholder_->setPalette(pal);
  placeholder_->installEventFilter(this);

  auto *placeholderLayout = new QVBoxLayout(placeholder_);
  placeholderLayout->setAlignment(Qt::AlignCenter);
  placeholderLayout->setSpacing(16);

  placeholderLabel_ = new QLabel(
      tr("No preview loaded\nClick here to load preview"), placeholder_);
  placeholderLabel_->setAlignment(Qt::AlignCenter);
  placeholderLabel_->setStyleSheet(
      QStringLiteral("color: #888888; font-size: 20px;"));
  placeholderLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
  placeholderLayout->addWidget(placeholderLabel_);

  stack_->addWidget(placeholder_); // index 0: placeholder
  stack_->addWidget(video_);       // index 1: video
  stack_->setMinimumSize(320, 180);

  seekSlider_->setRange(0, 0);
  seekSlider_->setEnabled(false);
  seekSlider_->setStyleSheet(QStringLiteral("QSlider::groove:horizontal {"
                                            " height: 18px;"
                                            " background: #444444;"
                                            " border-radius: 9px;"
                                            "}"
                                            "QSlider::sub-page:horizontal {"
                                            " background: #888888;"
                                            " border-radius: 9px;"
                                            "}"
                                            "QSlider::handle:horizontal {"
                                            " width: 22px;"
                                            " height: 22px;"
                                            " margin: -2px 0;"
                                            " border-radius: 11px;"
                                            " background: #cccccc;"
                                            "}"));
  seekSlider_->installEventFilter(this);

  playPauseButton_->setIcon(playIcon_);
  playPauseButton_->setIconSize(QSize(20, 20));
  playPauseButton_->setFixedSize(36, 30);
  playPauseButton_->setEnabled(false);
  video_->installEventFilter(this);

  overlayLabel_ = new QLabel(this);
  overlayLabel_->setAlignment(Qt::AlignCenter);
  overlayLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
  overlayLabel_->hide();

  overlayEffect_ = new QGraphicsOpacityEffect(overlayLabel_);
  overlayEffect_->setOpacity(0.0);
  overlayLabel_->setGraphicsEffect(overlayEffect_);

  overlayAnim_ = new QPropertyAnimation(overlayEffect_, "opacity", this);
  overlayAnim_->setDuration(450);
  overlayAnim_->setStartValue(0.9);
  overlayAnim_->setEndValue(0.0);
  connect(overlayAnim_, &QPropertyAnimation::finished, overlayLabel_,
          &QWidget::hide);

  overlayFadeTimer_ = new QTimer(this);
  overlayFadeTimer_->setSingleShot(true);
  overlayFadeTimer_->setInterval(600);
  connect(overlayFadeTimer_, &QTimer::timeout, overlayAnim_,
          [this]() { overlayAnim_->start(); });

  dotTimer_ = new QTimer(this);
  dotTimer_->setInterval(500);
  connect(dotTimer_, &QTimer::timeout, this, &PreviewWidget::onDotTimerTick);

  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(stack_, 1);

  auto *controls = new QHBoxLayout();
  controls->addWidget(playPauseButton_);
  controls->addWidget(seekSlider_, 1);
  controls->addWidget(timeLabel_);
  layout->addLayout(controls);

  connect(playPauseButton_, &QPushButton::clicked, this,
          &PreviewWidget::onPlayPauseClicked);
  connect(player_, &QMediaPlayer::positionChanged, this,
          &PreviewWidget::onPositionChanged);
  connect(player_, &QMediaPlayer::durationChanged, this,
          &PreviewWidget::onDurationChanged);
  connect(player_, &QMediaPlayer::playbackStateChanged, this,
          &PreviewWidget::onPlaybackStateChanged);
  connect(seekSlider_, &QSlider::sliderPressed, this,
          [this]() { sliderPressed_ = true; });
  connect(seekSlider_, &QSlider::sliderReleased, this, [this]() {
    sliderPressed_ = false;
    player_->setPosition(seekSlider_->value());
  });
  connect(seekSlider_, &QSlider::sliderMoved, this,
          &PreviewWidget::onSliderMoved);
}

PreviewWidget::~PreviewWidget() = default;

void PreviewWidget::loadFile(const QString &path) {
  dotTimer_->stop();
  invalidInputs_ = false;
  stop();
  if (path.isEmpty()) {
    clear();
    return;
  }
  hasMedia_ = false;
  stale_ = false;
  showingThumbnail_ = true;
  player_->setSource(QUrl::fromLocalFile(path));
  player_->play();
}

void PreviewWidget::clear() {
  dotTimer_->stop();
  invalidInputs_ = false;
  showingThumbnail_ = false;
  stop();
  player_->setSource(QUrl());
  hasMedia_ = false;
  stale_ = false;
  stack_->setCurrentIndex(0);
  placeholderLabel_->setText(
      tr("No preview loaded\nClick here to load preview"));
  seekSlider_->setRange(0, 0);
  timeLabel_->setText(QStringLiteral("0:00 / 0:00"));
  setControlsEnabled(false);
}

void PreviewWidget::stop() {
  showingThumbnail_ = false;
  if (player_->playbackState() != QMediaPlayer::StoppedState) {
    player_->stop();
  }
}

void PreviewWidget::setStale(bool stale) {
  stale_ = stale;
  if (!hasMedia_) {
    return;
  }
  if (stale) {
    stop();
    setControlsEnabled(false);
    stack_->setCurrentIndex(0);
    placeholderLabel_->setText(
        tr("Settings changed — generate a new preview\nClick here to reload"));
  } else {
    stack_->setCurrentIndex(1);
    setControlsEnabled(true);
  }
}

void PreviewWidget::setLoading(bool loading) {
  if (loading) {
    showingThumbnail_ = false;
    if (player_->playbackState() != QMediaPlayer::StoppedState) {
      player_->pause();
    }
    stack_->setCurrentIndex(0);
    dotCount_ = 0;
    placeholderLabel_->setText(tr("Generating preview."));
    dotTimer_->start();
    setControlsEnabled(false);
  } else {
    dotTimer_->stop();
    if (hasMedia_) {
      if (!stale_) {
        stack_->setCurrentIndex(1);
        setControlsEnabled(true);
      } else {
        stack_->setCurrentIndex(0);
        placeholderLabel_->setText(
            tr("Settings changed — generate a new preview\nClick here to "
               "reload"));
      }
    } else {
      placeholderLabel_->setText(
          tr("No preview loaded\nClick here to load preview"));
    }
  }
}

void PreviewWidget::setStatusText(const QString &text) {
  dotTimer_->stop();
  stop();
  stack_->setCurrentIndex(0);
  placeholderLabel_->setText(text);
}

void PreviewWidget::setControlsEnabled(bool enabled) {
  playPauseButton_->setEnabled(enabled);
  seekSlider_->setEnabled(enabled);
}

void PreviewWidget::onPlayPauseClicked() {
  if (!hasMedia_) {
    return;
  }
  if (player_->playbackState() == QMediaPlayer::PlayingState) {
    player_->pause();
  } else {
    player_->play();
  }
}

void PreviewWidget::onPositionChanged(qint64 position) {
  if (!sliderPressed_) {
    seekSlider_->setValue(static_cast<int>(position));
  }
  updateTimeLabel(position, player_->duration());
}

void PreviewWidget::onDurationChanged(qint64 duration) {
  seekSlider_->setRange(0, static_cast<int>(duration));
  updateTimeLabel(player_->position(), duration);
}

void PreviewWidget::onSliderMoved(int position) {
  updateTimeLabel(position, player_->duration());
}

void PreviewWidget::onPlaybackStateChanged(QMediaPlayer::PlaybackState state) {
  if (showingThumbnail_ && state == QMediaPlayer::PlayingState) {
    player_->pause();
    player_->setPosition(0);
    showingThumbnail_ = false;
    hasMedia_ = true;
    stack_->setCurrentIndex(1);
    if (!stale_) {
      setControlsEnabled(true);
    } else {
      stack_->setCurrentIndex(0);
      placeholderLabel_->setText(
          tr("Settings changed — generate a new preview\nClick here to "
             "reload"));
    }
    return;
  }
  playPauseButton_->setIcon(state == QMediaPlayer::PlayingState ? pauseIcon_
                                                                : playIcon_);
}

void PreviewWidget::updateTimeLabel(qint64 position, qint64 duration) {
  timeLabel_->setText(QStringLiteral("%1 / %2").arg(formatTime(position),
                                                    formatTime(duration)));
}

void PreviewWidget::onDotTimerTick() {
  dotCount_ = (dotCount_ % 3) + 1;
  placeholderLabel_->setText(tr("Generating preview") +
                             QString(dotCount_, QLatin1Char('.')));
}

void PreviewWidget::showClickOverlay(bool nowPausing) {
  overlayAnim_->stop();
  overlayFadeTimer_->stop();
  overlayLabel_->setPixmap(makeOverlayPixmap(!nowPausing));
  overlayLabel_->setGeometry(stack_->geometry());
  overlayLabel_->raise();
  overlayEffect_->setOpacity(0.9);
  overlayLabel_->show();
  overlayFadeTimer_->start();
}

void PreviewWidget::setPreviewClickEnabled(bool enabled) {
  previewClickEnabled_ = enabled;
  placeholder_->setCursor(enabled ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

void PreviewWidget::setInputsValid(bool valid) {
  if (dotTimer_->isActive()) {
    return;
  }

  if (!hasMedia_) {
    placeholderLabel_->setText(
        valid ? tr("No preview loaded\nClick here to load preview")
              : tr("Select valid input files to load a preview"));
    return;
  }

  if (!valid && !invalidInputs_) {
    invalidInputs_ = true;
    stack_->setCurrentIndex(0);
    placeholderLabel_->setText(
        tr("Select valid input files to load a preview"));
  } else if (valid && invalidInputs_) {
    invalidInputs_ = false;
    if (stale_) {
      stack_->setCurrentIndex(0);
      placeholderLabel_->setText(
          tr("Settings changed — generate a new preview\nClick here to "
             "reload"));
    } else {
      stack_->setCurrentIndex(1);
      setControlsEnabled(true);
    }
  }
}

bool PreviewWidget::eventFilter(QObject *obj, QEvent *event) {
  if (obj == placeholder_ && event->type() == QEvent::MouseButtonRelease &&
      previewClickEnabled_) {
    emit previewRequested();
    return false;
  }

  if (obj == seekSlider_ && event->type() == QEvent::MouseButtonPress &&
      seekSlider_->isEnabled()) {
    auto *me = static_cast<QMouseEvent *>(event);
    int val = QStyle::sliderValueFromPosition(
        seekSlider_->minimum(), seekSlider_->maximum(), me->pos().x(),
        seekSlider_->width());
    seekSlider_->setValue(val);
    sliderPressed_ = true;
    player_->setPosition(val);
    return false;
  }

  if (obj == video_ && event->type() == QEvent::MouseButtonRelease &&
      hasMedia_ && !stale_) {
    const bool wasPlaying =
        player_->playbackState() == QMediaPlayer::PlayingState;
    onPlayPauseClicked();
    showClickOverlay(wasPlaying);
    return false;
  }

  return QWidget::eventFilter(obj, event);
}

} // namespace ui
