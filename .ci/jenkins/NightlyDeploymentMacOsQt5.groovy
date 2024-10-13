pipeline {

  agent {
    label 'macos'
  }

  options {
    ansiColor('xterm')
    timestamps()
    timeout(45)
    skipDefaultCheckout true
    disableConcurrentBuilds abortPrevious: true
    buildDiscarder logRotator(numToKeepStr: '2')
  }

  triggers {
    // Each day between 0am and 6am, only if changes in Git.
    pollSCM 'H H(0-6) * * *'
  }

  environment {
     CMAKE_PREFIX_PATH = '/opt/Qt/5.15.2/clang_64/'
  }

  stages {
    stage('Checkout') {
      steps {
        git branch: 'master', url: 'https://github.com/Komet/MediaElch.git'
      }
    }
    stage('Build macOS Qt5') {
      steps {
        sh './.ci/macOS/build_macOS_release_Qt5.sh --no-confirm'
      }
    }
    stage('Package DMG') {
      steps {
        sh './.ci/macOS/package_macOS_Qt5.sh --no-confirm'
      }
    }
    stage('Upload DMG') {
      steps {
        withCredentials([sshUserPrivateKey(usernameVariable: 'ssh_user', credentialsId: 'mediaelch-downloads-ssh', keyFileVariable: 'keyfile')]) {
          sh '''
            if [ ! -d "${HOME}/.ssh" ] || ! grep 'ameyering.de' "${HOME}/.ssh/known_hosts" > /dev/null; then
              mkdir -p "${HOME}/.ssh"
              ssh-keyscan -H ameyering.de >> "${HOME}/.ssh/known_hosts"
            fi
            scp -i "${keyfile}" *.dmg ${ssh_user}@ameyering.de:compose_server/mediaelch_downloads/lighttpd_htdocs/snapshots/macOS_X/
            '''
        }
      }
    }
  }

  post {
    cleanup {
      sh 'rm -f *.dmg'
    }
  }
}
