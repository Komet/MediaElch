#!/usr/bin/env groovy

// Pipeline Script for Creating our Docker Images
//
// Jenkins requires these plugins:
//  - <https://plugins.jenkins.io/docker-workflow/>


pipeline {

  agent any

  options {
    ansiColor('xterm')
    timeout(60)
  }

  triggers {
    // Each Friday between 0am and 6am.
    cron 'H H(0-6) * * 5'
  }

  stages {

    stage('Checkout') {
      steps {
        git branch: 'master', url: 'https://github.com/Komet/MediaElch.git'
      }
    }

    stage('MediaElch CI Image') {
      environment {
        IMAGE_NAME = 'mediaelch/mediaelch-ci-appimage:latest'
        DOCKERFILE = 'Dockerfile.ci.linux'
      }
      steps {
        dir('.ci/docker') {
          script {
            docker.withRegistry('https://registry.hub.docker.com/', 'dockerhub') {
              docker.build("${IMAGE_NAME}", "--pull -f ${DOCKERFILE} .").push()
            }
          }
        }
      }
    }

    stage('MediaElch AppImage Build Image') {
      environment {
        IMAGE_NAME = 'mediaelch/mediaelch-ci-appimage:latest'
        DOCKERFILE = 'Dockerfile.build-ubuntu-16.04'
      }
      steps {
        dir('.ci/docker') {
          script {
            docker.withRegistry('https://registry.hub.docker.com/', 'dockerhub') {
              docker.build("${IMAGE_NAME}", "--pull -f ${DOCKERFILE} .").push()
            }
          }
        }
      }

    }
  }

}
