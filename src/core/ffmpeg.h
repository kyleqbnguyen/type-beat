#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

namespace core::ffmpeg {

class Renderer : public QObject {
  Q_OBJECT

public:
  explicit Renderer(QObject *parent = nullptr);

  void render(const QString &visualPath, const QString &audioPath,
              const QString &outputPath, double audioDuration);

signals:
  void finished();
  void errorOccurred(const QString &errorMessage);

private slots:
  void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void onProcessError(QProcess::ProcessError error);

private:
  static QString ffmpegPath();
  static bool isImageFile(const QString &path);
  QStringList buildImageArgs(const QString &visualPath,
                             const QString &audioPath,
                             const QString &outputPath) const;
  QStringList buildVideoArgs(const QString &visualPath,
                             const QString &audioPath,
                             const QString &outputPath,
                             double audioDuration) const;

  QProcess process_;
  bool errorEmitted_ = false;
};

} // namespace core::ffmpeg
