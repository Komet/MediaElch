#!/usr/bin/env groovy

// Pipeline Script for running the Coverity scan for MediaElch.
// Note that the Coverity package is part of our Jenkins instance.
// We can't download it on-demand and since it's 1.1GB, I don't want
// to download it too often.
//
// Jenkins requires these plugins:
//  - <https://plugins.jenkins.io/docker-workflow/>

pipeline {

  agent {
    docker {
      image 'mediaelch/mediaelch-ci-linux:latest'
      alwaysPull true
    }
  }

  options {
    ansiColor('xterm')
    timestamps()
    timeout(60)
    skipDefaultCheckout true
    disableConcurrentBuilds abortPrevious: true
    buildDiscarder logRotator(numToKeepStr: '2')
  }

  triggers {
    // Once a month (on the first) between 0am and 6am.
    cron 'H H(0-6) 1 * *'
  }

  stages {
    stage('Checkout') {
      steps {
        checkout scmGit(
          branches: [[name: '*/master']],
          extensions: [[$class: 'RelativeTargetDirectory', relativeTargetDir: 'MediaElch']],
          userRemoteConfigs: [[url: 'https://github.com/Komet/MediaElch.git']]
        )
      }
    }

    stage('Coverity Scan') {
      steps {
        withCredentials([file(credentialsId: 'curl-jenkins-binaries', variable: 'PASS')]) {
          sh '''
            JOB_DIR="$(pwd)"

            COVERITY=cov-analysis-linux64-2023.6.2.tar.gz
            COVERITY_DIR=cov-analysis-linux64-2023.6.2

            if [ ! -d "${COVERITY_DIR}" ]; then
              rm -f ${COVERITY} # ensure tar.gz does not exist
              curl --digest --config "${PASS}" \
                  --output "${COVERITY}" \
                  --no-progress-meter \
                  "https://files.ameyering.de/jenkins/${COVERITY}"
              tar -xf ${COVERITY}
              rm -f ${COVERITY}
            fi

            export PATH="$(pwd)/${COVERITY_DIR}/bin/:${PATH}"

            cd MediaElch
            git submodule update --init

            mkdir -p build/coverity
            cd build/coverity

            cmake -S ../.. -B . -DENABLE_TESTS=ON -DMEDIAELCH_FORCE_QT6=ON
            cov-build --dir cov-int make -j 3
            if grep "compilation units (100%) successfully" cov-int/build-log.txt; then
                tar caf myproject.xz cov-int
                mv myproject.xz "${JOB_DIR}/"
            fi
          '''
        }
      }
    }

    stage('Coverity Scan Upload') {
      steps {
        withCredentials([usernamePassword(credentialsId: 'coverity-token', passwordVariable: 'COVERITY_TOKEN', usernameVariable: 'COVERITY_MAIL')]) {
          sh '''
            MEDIAELCH_VERSION="$(grep AppVersionStr MediaElch/Version.h | grep --only-matching -e '[0-9]\\+.[0-9]\\+.[0-9]\\+')"
            curl --no-progress-meter \
              --form token="${COVERITY_TOKEN}" \
              --form email=${COVERITY_MAIL} \
              --form file=@myproject.xz \
              --form version="${MEDIAELCH_VERSION}" \
              --form description="MediaElch coverity CI scan for version ${MEDIAELCH_VERSION}" \
              https://scan.coverity.com/builds?project=Komet%2FMediaElch
            '''
        }
      }
    }
  }

  post {
    cleanup {
      // Don't remove Coverity directory.
      sh 'rm -rf MediaElch'
    }
  }

}
