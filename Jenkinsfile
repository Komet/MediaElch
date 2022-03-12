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
  }

  stages {

    stage('Lint') {
      failFast true
      parallel {
        stage('Shellcheck') {
          steps {
            sh './scripts/run_shellcheck.sh'
          }
        }
        stage('clang-format') {
          steps {
            sh '''
              ./scripts/run_clang_format.sh
              git diff --diff-filter=M --color | cat
              git diff --diff-filter=M --quiet || (echo "Found unformatted C++ files! Use clang-format!"; exit 1)
              '''
          }
        }
        stage('CMake Format') {
          steps {
            sh '''
              ./scripts/run_cmake_format.sh
              git diff --diff-filter=M --color | cat
              git diff --diff-filter=M --quiet || (echo "Found unformatted CMakeLists.txt! Use cmake-format!"; exit 1)
              '''
          }
        }
        stage('cppcheck') {
          steps {
            // Currently allow cppcheck to fail due to false positives
            sh './scripts/run_cppcheck.sh || echo "Cppcheck Failed"'
          }
        }
      }
    }

    stage('Build') {
      steps {
        sh '''
          cmake --preset ci
          cmake --build --preset ci
          '''
      }
    }
    stage('Test') {
      steps {
        sh 'mkdir -p ./build/ci/reports'
        sh 'xvfb-run ./build/ci/test/unit/mediaelch_unit -r junit --use-colour yes --warn NoTests --out build/ci/reports/mediaelch_unit.xml'
        sh 'xvfb-run ./build/ci/test/integration/mediaelch_test_integration -r junit --durations yes --use-colour yes --warn NoTests --resource-dir ./test/resources --temp-dir ./build/ci/test/resources --out ./build/ci/reports/mediaelch_test_integration.xml'
      }
    }
  }
  post {
    always {
      recordIssues enabledForFailure: true, tool: gcc()
      recordIssues enabledForFailure: true, tool: cmake()
      junit 'build/reports/*.xml'
    }
    cleanup {
      // Delete report and binaries
      sh '''
        rm -rf ./build/ci/reports
        rm -f ./build/ci/test/unit/mediaelch_unit
        rm -f ./build/ci/test/integration/mediaelch_test_integration
        '''
    }
  }
}
