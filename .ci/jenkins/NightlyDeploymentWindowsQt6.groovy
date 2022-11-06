pipeline {

  agent {
    docker {
      image 'mediaelch/mediaelch-ci-win:qt6'
      alwaysPull true
    }
  }

  options {
    ansiColor('xterm')
    timestamps()
    timeout(30)
    skipDefaultCheckout true
    disableConcurrentBuilds abortPrevious: true
    buildDiscarder logRotator(numToKeepStr: '3')
  }

  triggers {
    // Each day between 0am and 6am, only if changes in Git.
    pollSCM 'H H(0-6) * * *'
  }

  stages {
    stage('Checkout') {
      steps {
        git branch: 'master', url: 'https://github.com/Komet/MediaElch.git'
      }
    }
    stage('Build Windows MXE Qt6') {
      steps {
        sh 'QT_MAJOR_VERSION=6 ./.ci/win/build_windows_release.sh --no-confirm'
      }
    }
    stage('Package ZIP') {
      steps {
        sh 'QT_MAJOR_VERSION=6 ./.ci/win/package_windows.sh'
      }
    }
    stage('Upload ZIP') {
      steps {
        withCredentials([sshUserPrivateKey(usernameVariable: 'ssh_user', credentialsId: 'mediaelch-downloads-ssh', keyFileVariable: 'keyfile')]) {
          sh '''
            if [ ! -d "${HOME}/.ssh" ] || ! grep 'ameyering.de' "${HOME}/.ssh/known_hosts" > /dev/null; then
              mkdir -p "${HOME}/.ssh"
              ssh-keyscan -H ameyering.de >> "${HOME}/.ssh/known_hosts"
            fi
            scp -i "${keyfile}" *.zip ${ssh_user}@ameyering.de:compose_server/mediaelch_downloads/lighttpd_htdocs/snapshots/Windows_10_or_later/
            '''
        }
      }
    }
  }

  post {
    cleanup {
      sh 'rm -f *.zip'
    }
  }
}
