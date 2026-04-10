#pragma once

#include <QIcon>
#include <QMediaPlayer>
#include <QString>
#include <QWidget>

class QAudioOutput;
class QEvent;
class QGraphicsOpacityEffect;
class QLabel;
class QPropertyAnimation;
class QPushButton;
class QSlider;
class QStackedWidget;
class QTimer;
class QVideoWidget;

namespace ui {

class PreviewWidget : public QWidget {
  Q_OBJECT

public:
  explicit PreviewWidget(QWidget *parent = nullptr);
  ~PreviewWidget() override;

  void loadFile(const QString &path);
  void clear();
  void stop();

  void setStale(bool stale);
  [[nodiscard]] bool isStale() const { return stale_; }

  void setLoading(bool loading);
  void setStatusText(const QString &text);
  void setPreviewClickEnabled(bool enabled);

signals:
  void previewRequested();

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
  void onPlayPauseClicked();
  void onPositionChanged(qint64 position);
  void onDurationChanged(qint64 duration);
  void onSliderMoved(int position);
  void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
  void onDotTimerTick();

private:
  void updateTimeLabel(qint64 position, qint64 duration);
  void setControlsEnabled(bool enabled);
  void showClickOverlay(bool nowPausing);

  QMediaPlayer *player_;
  QAudioOutput *audio_;
  QStackedWidget *stack_;
  QWidget *placeholder_;
  QLabel *placeholderLabel_;
  QVideoWidget *video_;
  QLabel *overlayLabel_;
  QGraphicsOpacityEffect *overlayEffect_;
  QPropertyAnimation *overlayAnim_;
  QTimer *overlayFadeTimer_;
  QTimer *dotTimer_;
  QPushButton *playPauseButton_;
  QSlider *seekSlider_;
  QLabel *timeLabel_;

  QIcon playIcon_;
  QIcon pauseIcon_;

  bool stale_ = false;
  bool hasMedia_ = false;
  bool sliderPressed_ = false;
  bool showingThumbnail_ = false;
  bool previewClickEnabled_ = false;
  int dotCount_ = 0;
};

} // namespace ui
