#!/usr/bin/env groovy

// Pipeline Script for Creating our MXE Docker Image
//
// Jenkins requires these plugins:
//  - <https://plugins.jenkins.io/docker-workflow/>
//
// Note:
//  - This is not part of BuildDockerImages.groovy because building
//    this image is unstable.  MXE builds may succeed or may fail for
//    unspecific reasons. I had some internal compiler errors in the past.
//    This build can also take a few hours.

pipeline {

  agent any

  options {
    ansiColor('xterm')
    timestamps()
  }

  triggers {
    // Each Friday between 0am and 6am.
    cron 'H H(0-6) * * 5'
  }

  stages {

    stage('MediaElch Windows MXE Build Image') {
      environment {
        IMAGE_NAME = 'mediaelch/mediaelch-ci-win:latest'
        DOCKERFILE = 'Dockerfile.ci.windows'
      }
      steps {
        dir('.ci/docker') {
          script {
            docker.withRegistry('https://registry.hub.docker.com/', 'dockerhub') {
              docker.build("${IMAGE_NAME}", "--no-cache --pull -f ${DOCKERFILE} .").push()
            }
          }
        }
      }

    }
  }

}
