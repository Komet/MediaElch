# Transifex / i18n

This page describes how to update translations locally and on transifex.com

You need to install the [Transifex client](https://docs.transifex.com/client/installing-the-client)

```sh
# You need Qt5 Tools for lupdate and lrelease
sudo apt-get install qt5-default qt5-qmake qttools5-dev-tools

export TX_TOKEN=<your_Transifex_API_token>
cd MediaElch
./scripts/update_translation_source.sh
```
