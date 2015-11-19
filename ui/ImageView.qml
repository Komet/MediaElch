import QtQuick 2.0
import QtQml.Models 2.1
import QtQuick.Controls 1.3

Item {
    id: root

    FontLoader { source: "qrc:/fonts/Pe-icon-7-stroke.ttf" }

    ScrollView {
        id: scrollView
        anchors.fill: parent
        frameVisible: false
        enabled: !loading

        GridView {
            id: gridView
            cellWidth: scrollView.viewport.width / Math.floor(scrollView.viewport.width/200)
            cellHeight: cellWidth+20
            interactive: false
            cacheBuffer: cellHeight * 10
            displaced: Transition {
                NumberAnimation { properties: "x,y"; easing.type: Easing.OutQuad }
            }

            model: DelegateModel {
                id: visualModel
                model: album ? album.bookletModel : undefined
                delegate: MouseArea {
                    id: delegateRoot

                    property int visualIndex: DelegateModel.itemsIndex

                    width: gridView.cellWidth - 20
                    height: gridView.cellHeight - 40
                    drag.target: img
                    cursorShape: Qt.SizeAllCursor
                    onPressed: globalDrop.enabled = false
                    onReleased: globalDrop.enabled = true

                    Image {
                        id: img
                        width: gridView.cellWidth - 40
                        height: gridView.cellHeight - 60
                        asynchronous: true
                        smooth: true
                        anchors {
                            horizontalCenter: parent.horizontalCenter;
                            verticalCenter: parent.verticalCenter
                        }
                        source: album && model.imageId ? "image://album/booklet/" + album.artistObj.modelItem.childNumber() + "/" + album.modelItem.childNumber() + "/" + model.imageId : ""
                        fillMode: Image.PreserveAspectFit
                        opacity: model.deletion ? 0.3 : 1

                        Drag.active: delegateRoot.drag.active
                        Drag.source: delegateRoot
                        Drag.hotSpot.x: img.width/2
                        Drag.hotSpot.y: img.height/2

                        states: [
                            State {
                                when: img.Drag.active
                                ParentChange {
                                    target: img
                                    parent: gridView
                                }

                                AnchorChanges {
                                    target: img;
                                    anchors.horizontalCenter: undefined;
                                    anchors.verticalCenter: undefined
                                }
                            }
                        ]

                        Rectangle {
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.rightMargin: -width/2
                            anchors.topMargin: -height/2
                            color: "#000000"
                            width: 20
                            height: 20
                            radius: width/2
                            Text {
                                anchors.fill: parent
                                font.family: "Pe-icon-7-stroke"
                                color: "#ffffff"
                                font.pixelSize: 24
                                text: "\ue680"
                                renderType: isOsx ? Text.NativeRendering : Text.QtRendering
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                            }
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: model.deletion = !model.deletion
                            }
                        }
                        Text {
                            anchors.top: img.bottom
                            anchors.left: img.left
                            anchors.topMargin: 4
                            renderType: isOsx ? Text.NativeRendering : Text.QtRendering
                            color: "#666666"
                            font.family: "Pe-icon-7-stroke"
                            font.pixelSize: 16
                            text: "\ue618"
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: imageWidget.zoomImage(album.artistObj.modelItem.childNumber(), album.modelItem.childNumber(), model.imageId)
                            }
                        }
                        Text {
                            id: btnCut
                            anchors.top: img.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.topMargin: 2
                            renderType: isOsx ? Text.NativeRendering : Text.QtRendering
                            color: "#666666"
                            font.family: "Pe-icon-7-stroke"
                            font.pixelSize: 20
                            text: "\ue697"
                            width: 16
                            height: 16
                            rotation: 270
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: imageWidget.cutImage(album.artistObj.modelItem.childNumber(), album.modelItem.childNumber(), model.imageId)
                            }
                        }
                        Text {
                            anchors.top: img.bottom
                            anchors.right: img.right
                            anchors.topMargin: 6
                            text: img.sourceSize.width + "x" + img.sourceSize.height
                            font.pixelSize: 10
                            renderType: isOsx ? Text.NativeRendering : Text.QtRendering
                            color: "#666666"
                        }
                    }

                    DropArea {
                        anchors { fill: parent; margins: 15 }
                        onEntered: {
                            if (drag.source) {
                                album.bookletModel.move(drag.source.visualIndex, delegateRoot.visualIndex)
                            }
                        }
                    }
                }
            }
        }
    }

    DropArea {
        id: globalDrop
        anchors.fill: scrollView
        enabled: !loading
        onEntered: {
            if (!drag.hasUrls) {
                drag.accepted = false
                return;
            }

            var endsWithJpg = /jpg$/i;
            var endsWithJpeg = /jpeg$/i;
            for (var i=0 ; i<drag.urls.length ; i++) {
                if (!endsWithJpeg.test(drag.urls[i]) && !endsWithJpg.test(drag.urls[i]))  {
                    drag.accepted = false
                    return;
                }
            }
            drag.accepted = true
        }
        onDropped: {
            if (!drop.hasUrls) {
                drop.accepted = false
                return;
            }
            var urls = []
            var endsWithJpg = /jpg$/i;
            var endsWithJpeg = /jpeg$/i;
            for (var i=0 ; i<drop.urls.length ; i++) {
                if (!endsWithJpeg.test(drop.urls[i]) && !endsWithJpg.test(drop.urls[i]))  {
                    drop.accepted = false
                    return;
                }
                urls.push(drop.urls[i])
            }
            drop.accepted = true
            imageWidget.imagesDropped(urls)
        }
    }

    Rectangle {
        color: "#40000000"
        anchors.fill: parent
        opacity: loading ? 1 : 0
        enabled: loading
        MouseArea {
            anchors.fill: parent
            onClicked: {}
            onWheel: {}
        }
    }
}

