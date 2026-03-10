#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

namespace core::ffprobe {

class DurationProbe : public QObject {
  Q_OBJECT

public:
  explicit DurationProbe(QObject *parent = nullptr);

  void probe(const QString &filePath);

signals:
  void durationReady(double seconds);
  void errorOccurred(const QString &errorMessage);

private slots:
  void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void onProcessError(QProcess::ProcessError error);

private:
  static QString ffprobePath();

  QProcess process_;
  bool errorEmitted_ = false;
};

} // namespace core::ffprobe
