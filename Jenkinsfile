pipeline {

  agent {
    docker {
      image 'mediaelch/mediaelch-ci-linux:latest'
      alwaysPull true
    }
  }

  options {
    ansiColor('xterm')
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
            sh './scripts/run_clang_format.sh'
            sh 'git diff --diff-filter=M --color | cat'
            sh 'git diff --diff-filter=M --quiet || (echo "Found unformatted C++ files! Use clang-format!"; exit 1)'
          }
        }
        stage('CMake Format') {
          steps {
            sh './scripts/run_cmake_format.sh'
            sh 'git diff --diff-filter=M --color | cat'
            sh 'git diff --diff-filter=M --quiet || (echo "Found unformatted CMakeLists.txt! Use cmake-format!"; exit 1)'
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

    stage('Build and Test') {
      failFast true
      parallel {
/*
        stage('Windows using MXE') {
          when { branch 'skip-for-now' }
          agent {
            docker {
              image 'mediaelch/mediaelch-ci-win:latest'
            }
          }
          steps {
            sh '.ci/win/build_windows.sh'
          }
        }
*/
        stage('Linux') {
          stages {
            stage('Build') {
              steps {
                sh 'mkdir -p build'
                sh 'cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -DDISABLE_UPDATER=ON -DENABLE_COLOR_OUTPUT=ON -GNinja'
                sh 'cmake --build build --config Debug -j 2'
              }
            }
            stage('Test') {
              steps {
                sh 'rm -rf build/reports'
                sh 'mkdir -p build/reports'
                sh 'xvfb-run ./build/test/unit/mediaelch_unit -r junit --use-colour yes --warn NoTests --out build/reports/mediaelch_unit.xml'
                sh 'xvfb-run ./build/test/integration/mediaelch_test_integration -r junit --durations yes --use-colour yes --warn NoTests --resource-dir ./test/resources --temp-dir ./build/test/resources --out build/reports/mediaelch_test_integration.xml'
              }
            }
          }
          post {
            always {
              recordIssues enabledForFailure: true, tool: gcc()
              recordIssues enabledForFailure: true, tool: cmake()
              junit 'build/reports/*.xml'
            }
          }
        }
      }
    }

    stage('Deploy') {
      when { branch 'master' }
      steps {
        sh 'echo "Not implemented, yet"'
      }
    }
  }
}
