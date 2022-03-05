pipeline {

  agent any

  options {
    ansiColor('xterm')
  }

  triggers {
    // Each day between 0am and 6am.
    cron 'H H(0-6) * * *'
  }

  stages {

    stage('Checkout') {
      steps {
        git branch: 'master', url: 'https://github.com/Komet/MediaElch.git'
      }
    }

    stage('Linux AppImage') {
      agent {
        docker {
          image 'mediaelch/mediaelch-ci-appimage:latest'
          alwaysPull true
          reuseNode true
        }
      }
      stages {
        stage('Build') {
          steps {
            // Note: /bin/sh is used, i.e. no Bash
            sh '''
              set +e
              . /opt/qt512/bin/qt512-env.sh
              set -e
              ./.ci/linux/build_linux_release.sh --no-confirm
              '''
          }
        }
        stage('Package') {
          steps {
            // Note: /bin/sh is used, i.e. no Bash
            sh '''
              set +e
              . /opt/qt512/bin/qt512-env.sh
              set -e
              ./.ci/linux/package_linux_appimage.sh --no-confirm
              '''
          }
        }
        stage('Upload') {
          steps {
            withCredentials([sshUserPrivateKey(usernameVariable: 'ssh_user', credentialsId: "mediaelch-downloads-ssh", keyFileVariable: 'keyfile')]) {
              sh '''
                if [ ! -d "${HOME}/.ssh" ] || ! grep 'ameyering.de' "${HOME}/.ssh/known_hosts" > /dev/null; then
                  mkdir -p "${HOME}/.ssh"
                  ssh-keyscan -H ameyering.de >> "${HOME}/.ssh/known_hosts"
                fi
                scp -i "${keyfile}" *.AppImage ${ssh_user}@ameyering.de:compose_server/mediaelch_downloads/lighttpd_htdocs/snapshots/Linux/
                '''
            }
          }
        }
      }
    }
    stage('Windows ZIP') {
      agent {
        docker {
          image 'mediaelch/mediaelch-ci-win:latest'
          alwaysPull true
          reuseNode true
        }
      }
      stages {
        stage('Build') {
          steps {
            sh './.ci/win/build_windows_release.sh --no-confirm'
          }
        }
        stage('Package') {
          steps {
            sh './.ci/win/package_windows.sh'
          }
        }
        stage('Upload') {
          steps {
            withCredentials([sshUserPrivateKey(usernameVariable: 'ssh_user', credentialsId: 'mediaelch-downloads-ssh', keyFileVariable: 'keyfile')]) {
              sh '''
                if [ ! -d "${HOME}/.ssh" ] || ! grep 'ameyering.de' "${HOME}/.ssh/known_hosts" > /dev/null; then
                  mkdir -p "${HOME}/.ssh"
                  ssh-keyscan -H ameyering.de >> "${HOME}/.ssh/known_hosts"
                fi
                scp -i "${keyfile}" *.zip ${ssh_user}@ameyering.de:compose_server/mediaelch_downloads/lighttpd_htdocs/snapshots/Windows/
                '''
            }
          }
        }
      }
    }
  }

  post {
    cleanup {
      sh 'rm -f *.AppImage *.zip *.dmg'
    }
  }
}
