#include "workers/Job.h"

#include <QTimer>

namespace mediaelch {
namespace worker {

Job::~Job()
{
    if (!m_isFinished) {
        emitFinished();
    }
}

void Job::start()
{
    QTimer::singleShot(0, this, &Job::doStart);
}

bool Job::kill()
{
    if (m_isFinished) {
        return true;
    }

    if (doKill()) {
        setError(ErrorCode::KilledJobError);
        emitFinished();
        return true;
    }

    return false;
}

bool Job::hasError() const
{
    return m_errorCode != 0;
}

int Job::errorCode() const
{
    return m_errorCode;
}

QString Job::errorString() const
{
    return m_errorString;
}

QString Job::errorText() const
{
    return m_errorText;
}

bool Job::isAutoDelete() const
{
    return m_isAutoDelete;
}

void Job::setAutoDelete(bool autoDelete)
{
    m_isAutoDelete = autoDelete;
}

bool Job::doKill()
{
    return false;
}

void Job::setError(int errorCode)
{
    m_errorCode = static_cast<ErrorCode>(errorCode);
}

void Job::setErrorString(QString msg)
{
    m_errorString = std::move(msg);
}

void Job::setErrorText(QString msg)
{
    m_errorText = std::move(msg);
}

void Job::emitFinished()
{
    MediaElch_Assert(!m_isFinished);
    m_isFinished = true;

    emit finished(this, QPrivateSignal());

    if (isAutoDelete()) {
        deleteLater();
    }
}

void Job::emitPercent(elch_ssize_t processed, elch_ssize_t total)
{
    if (total <= 0) {
        emit percentChanged(this, 0.0f, QPrivateSignal{});
        return;
    }
    const float old = m_percent;
    m_percent = 100.0f * (static_cast<float>(processed) / static_cast<float>(total));
    // Using epsilon of 0.05;
    if (m_percent >= old + 0.05f || m_percent <= old - 0.05f) {
        emit percentChanged(this, m_percent, QPrivateSignal{});
    }
}


} // namespace worker
} // namespace mediaelch
