# Transifex / i18n

__State__: last updated 2023-05-11

This page describes how to update translations locally and on transifex.com

You need to install the [Transifex client](https://developers.transifex.com/docs/cli#how-to-install-the-client)

```sh
# You need Qt5 Tools for lupdate and lrelease
# You can also try it with Qt6, but I haven't tested it, yet.
sudo apt-get install qt5-default qt5-qmake qttools5-dev-tools

export TX_TOKEN="<your_Transifex_API_token>"
cd MediaElch
./scripts/update_translation_source.sh
```
