// Copyright 2010-2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

/**
 * Overlay drawing test application.
 */

#include "../../overlay/overlay.h"

#include "IPCUtils.h"
#include "Timer.h"

#ifdef Q_OS_WIN
#	include "win.h"
#else
#	include <unistd.h>
#endif

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include <QWidget>
#include <QMainWindow>
#include <QApplication>

class OverlayWidget : public QWidget {
	Q_OBJECT

protected:
	QImage img;
	OverlayMsg om;
	QLocalSocket *qlsSocket;
	QTimer *qtTimer;
	QRect qrActive;
	QElapsedTimer qtWall;

	unsigned int iFrameCount;
	int iLastFpsUpdate;

	unsigned int uiWidth, uiHeight;

	void resizeEvent(QResizeEvent *);
	void paintEvent(QPaintEvent *);
	void init(const QSize &);
	void detach();

	void keyPressEvent(QKeyEvent *);

	int readClipImage();
protected slots:
	void connected();
	void disconnected();
	void readyRead();
	void error(QLocalSocket::LocalSocketError);
	void update();

public:
	OverlayWidget(QWidget *p = nullptr);
};

OverlayWidget::OverlayWidget(QWidget *p) : QWidget(p) {
	qlsSocket = nullptr;
	uiWidth = uiHeight = 0;

	setFocusPolicy(Qt::StrongFocus);
	setFocus();

	qtTimer = new QTimer(this);
	connect(qtTimer, SIGNAL(timeout()), this, SLOT(update()));
	qtTimer->start(100);
}

void OverlayWidget::keyPressEvent(QKeyEvent *evt) {
	evt->accept();
	qWarning("Keypress");
	detach();
}

void OverlayWidget::resizeEvent(QResizeEvent *evt) {
	QWidget::resizeEvent(evt);

	if (!img.isNull())
		img = img.scaled(evt->size().width(), evt->size().height());
	init(evt->size());
}

void OverlayWidget::paintEvent(QPaintEvent *) {
	if (!qlsSocket || qlsSocket->state() == QLocalSocket::UnconnectedState) {
		detach();

		qlsSocket = new QLocalSocket(this);
		connect(qlsSocket, SIGNAL(connected()), this, SLOT(connected()));
		connect(qlsSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
		connect(qlsSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
		connect(qlsSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), this,
				SLOT(error(QLocalSocket::LocalSocketError)));
#ifdef Q_OS_WIN
		qlsSocket->connectToServer(QLatin1String("MumbleOverlayPipe"));
#else
		QDir mumbleRuntimeDir = Mumble::getRuntimeDir();
		QString pipepath = mumbleRuntimeDir.absoluteFilePath(QLatin1String("MumbleOverlayPipe"));
		qWarning() << "connectToServer(" << pipepath << ")";
		qlsSocket->connectToServer(pipepath);
#endif
	}

	QPainter painter(this);
	painter.fillRect(0, 0, width(), height(), QColor(128, 0, 128));
	painter.setClipRect(qrActive);
	painter.drawImage(0, 0, img);
}

void OverlayWidget::init(const QSize &sz) {
	qWarning() << "Init" << sz.width() << sz.height();

	qtWall.start();
	iFrameCount    = 0;
	iLastFpsUpdate = 0;

	OverlayMsg m;
	m.omh.uiMagic  = OVERLAY_MAGIC_NUMBER;
	m.omh.uiType   = OVERLAY_MSGTYPE_INIT;
	m.omh.iLength  = sizeof(OverlayMsgInit);
	m.omi.uiWidth  = sz.width();
	m.omi.uiHeight = sz.height();

	if (qlsSocket && qlsSocket->state() == QLocalSocket::ConnectedState)
		qlsSocket->write(m.headerbuffer, sizeof(OverlayMsgHeader) + sizeof(OverlayMsgInit));

	img = QImage(sz.width(), sz.height(), QImage::Format_ARGB32_Premultiplied);
}

void OverlayWidget::detach() {
	if (qlsSocket) {
		qlsSocket->abort();
		qlsSocket->deleteLater();
		qlsSocket = nullptr;
	}
}

void OverlayWidget::connected() {
	qWarning() << "connected";

	OverlayMsg m;
	m.omh.uiMagic = OVERLAY_MAGIC_NUMBER;
	m.omh.uiType  = OVERLAY_MSGTYPE_PID;
	m.omh.iLength = sizeof(OverlayMsgPid);
#ifdef Q_OS_WIN
	m.omp.pid = GetCurrentProcessId();
#else
	m.omp.pid = getpid();
#endif
	qlsSocket->write(m.headerbuffer, sizeof(OverlayMsgHeader) + sizeof(OverlayMsgPid));

	om.omh.iLength = -1;

	init(size());
}

void OverlayWidget::disconnected() {
	qWarning() << "disconnected";

	QLocalSocket *qls = qobject_cast< QLocalSocket * >(sender());
	if (qls == qlsSocket) {
		detach();
	}
}

void OverlayWidget::error(QLocalSocket::LocalSocketError) {
	perror("error");
	disconnected();
}

void OverlayWidget::update() {
	++iFrameCount;

	float elapsed = static_cast< float >(qtWall.elapsed() - iLastFpsUpdate) / 1000.0f;

	if (elapsed > OVERLAY_FPS_INTERVAL) {
		struct OverlayMsg om;
		om.omh.uiMagic = OVERLAY_MAGIC_NUMBER;
		om.omh.uiType  = OVERLAY_MSGTYPE_FPS;
		om.omh.iLength = sizeof(struct OverlayMsgFps);
		om.omf.fps     = static_cast< int >(static_cast< float >(iFrameCount) / elapsed);

		if (qlsSocket && qlsSocket->state() == QLocalSocket::ConnectedState) {
			qlsSocket->write(reinterpret_cast< char * >(&om), sizeof(OverlayMsgHeader) + om.omh.iLength);
		}

		iFrameCount    = 0;
		iLastFpsUpdate = 0;
		qtWall.start();
	}

	QWidget::update();
}

int OverlayWidget::readClipImage() {
	int toRead = om.omb.w * OVERLAY_N_COLOUR_CHANNELS;
	int toReadTot = om.omb.h * toRead;
    //qWarning() << "toRead:" << toRead << "--- byteAvailable:" << qlsSocket->bytesAvailable();
	unsigned char *dst;
	int readTot = 0;
	for (unsigned int y = 0; y < om.omb.h; ++y) {
//	    if (qlsSocket->bytesAvailable() < toRead)
//	        sleep(1);
//		    return -1;
		dst = img.scanLine(y + om.omb.y) + om.omb.x * OVERLAY_N_COLOUR_CHANNELS;
		readTot += qlsSocket->read(reinterpret_cast< char * >(dst), om.omb.w * OVERLAY_N_COLOUR_CHANNELS);
	}
	qWarning() << "toReadTot:" << toReadTot << "--- readTot:" << readTot << "--- byteAvailable:" << qlsSocket->bytesAvailable();

	//memcpy(img.scanLine(0), dst, toReadTOt);
	//qrActive = QRect(om.oma.x, om.oma.y, om.oma.w, om.oma.h);
	return 0;
}

void OverlayWidget::readyRead() {
	QLocalSocket *qls = qobject_cast< QLocalSocket * >(sender());
	if (qls != qlsSocket) {
		qWarning() << "qls != qlsSocket";
		return;
	}

	while (true) {
		int ready = qlsSocket->bytesAvailable();
		qWarning() << "ready:" << ready;

		if (om.omh.iLength == -1) {
			if ((size_t)ready < sizeof(OverlayMsgHeader)) {
				qWarning() << "ready:" << ready << "--- sizeof(OverlayMsgHeader):" << sizeof(OverlayMsgHeader);
				break;
			} else {
				qlsSocket->read(reinterpret_cast< char * >(om.headerbuffer), sizeof(OverlayMsgHeader));
				if ((om.omh.uiMagic != OVERLAY_MAGIC_NUMBER) || (om.omh.iLength < 0)) {
					qWarning() << "uiMagic:" << om.omh.uiMagic << "--- iLength:" << om.omh.iLength;
					detach();
					return;
				}
				ready -= sizeof(OverlayMsgHeader);
			}
		}

		if (ready >= om.omh.iLength) {
			int length = qlsSocket->read(om.msgbuffer, om.omh.iLength);

			qWarning() << length << om.omh.uiType;

			if (length != om.omh.iLength) {
				qWarning() << "length != om.omh.iLength";
				detach();
				return;
			}

			switch (om.omh.uiType) {
				case OVERLAY_MSGTYPE_BLIT: {
					int toReadTot = om.omb.h * om.omb.w * OVERLAY_N_COLOUR_CHANNELS;
					if (ready <= toReadTot) {
						qWarning() << "1° ready:" << ready << "--- toReadTot:" << toReadTot;
						break;
					}
					OverlayMsgBlit *omb = &om.omb;
					length -= sizeof(OverlayMsgBlit);

					qWarning() << "BLIT" << omb->x << omb->y << omb->w << omb->h;

//					if (((omb->x + omb->w) > (unsigned int)img.width())
//						|| ((omb->y + omb->h) > (unsigned int)img.height()))
//						break;
					int ret = readClipImage();
					if (ret == -1)
						qWarning() << "readClipImage() failed";
					else
						qWarning() << "readClipImage() succeeded";
					update();
				} break;
				case OVERLAY_MSGTYPE_ACTIVE: {
					OverlayMsgActive *oma = &om.oma;

					qWarning() << "ACTIVE" << oma->x << oma->y << oma->w << oma->h;

					qrActive = QRect(oma->x, oma->y, oma->w, oma->h);
					update();
				}; break;
				default:
					break;
			}
			int toReadTot = om.omb.h * om.omb.w * OVERLAY_N_COLOUR_CHANNELS;
			if (om.omh.uiType == OVERLAY_MSGTYPE_BLIT && ready < toReadTot) {
				qWarning() << "2° ready:" << ready << "--- toReadTot:" << toReadTot;
				break;
			} else {
				om.omh.iLength = -1;
				ready -= length;
				qWarning() << "iLength:" << om.omh.iLength << "--- ready:" << ready;
			}
		} else {
			break;
		}
	}
}

class TestWin : public QObject {
	Q_OBJECT

protected:
	QWidget *qw;

public:
	TestWin();
};

TestWin::TestWin() {
	QMainWindow *qmw = new QMainWindow();
	qmw->setObjectName(QLatin1String("main"));

	OverlayWidget *ow = new OverlayWidget();
	qmw->setCentralWidget(ow);

	qmw->show();
	qmw->resize(1280, 720);

	QMetaObject::connectSlotsByName(this);
}

int main(int argc, char **argv) {
	QApplication a(argc, argv);

	TestWin t;

	return a.exec();
}

#include "OverlayTest.moc"
