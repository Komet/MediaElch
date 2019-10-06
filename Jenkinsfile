pipeline {
  agent {
    docker {
      image 'ubuntu:disco'
    }

  }
  stages {
    stage('Install') {
      steps {
        timestamps() {
          sh 'apt-get update'
          sh 'apt-get -y --no-install-recommends install g++ gcc build-essential git cmake wget libmediainfo-dev ffmpeg qt5-default qtmultimedia5-dev qtdeclarative5-dev libqt5opengl5 libqt5opengl5-dev'
          sh 'apt-get install -y --reinstall ca-certificates'
          sh 'cmake --version'
          sh 'g++ --version'
        }

      }
    }
    stage('Build') {
      steps {
        sh 'git config --global http.sslverify false'
        sh './travis-ci/docker/build-ubuntu.sh ubuntu-19.04'
      }
    }
  }
}
