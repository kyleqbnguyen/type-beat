#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

namespace core::ffmpeg {

class Renderer : public QObject {
  Q_OBJECT

public:
  explicit Renderer(QObject *parent = nullptr);
  ~Renderer() override;

  void render(const QString &visualPath, const QString &audioPath,
              const QString &outputPath);
  void cancel();
  [[nodiscard]] bool isRunning() const;
  [[nodiscard]] QString fullStderr() const;

signals:
  void finished();
  void errorOccurred(const QString &errorMessage);
  void progressUpdated(int percent);

private slots:
  void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void onProcessError(QProcess::ProcessError error);
  void onReadyReadStderr();

private:
  static QString ffmpegPath();
  static bool isImageFile(const QString &path);
  QStringList buildImageArgs(const QString &visualPath,
                             const QString &audioPath,
                             const QString &outputPath) const;
  QStringList buildVideoArgs(const QString &visualPath,
                             const QString &audioPath,
                             const QString &outputPath) const;
  static double parseDuration(const QString &output);
  static double parseTime(const QString &line);

  QProcess process_;
  bool errorEmitted_ = false;
  bool cancelled_ = false;
  double durationSeconds_ = 0.0;
  QString fullStderr_;
};

} // namespace core::ffmpeg
