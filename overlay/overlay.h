// Copyright 2005-2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_INTERNAL_OVERLAY_H_
#define MUMBLE_INTERNAL_OVERLAY_H_

// overlay message protocol version number
#define OVERLAY_MAGIC_NUMBER 0x00000006

#define OVERLAY_N_COLOUR_CHANNELS 4

struct OverlayMsgHeader {
	unsigned int uiMagic;
	int iLength;
	unsigned int uiType;
};

#define OVERLAY_MSGTYPE_INIT 0
struct OverlayMsgInit {
	unsigned int uiWidth;
	unsigned int uiHeight;
};

/* #define OVERLAY_MSGTYPE_SHMEM 1
struct OverlayMsgShmem {
	char a_cName[2048];
};
*/

#define OVERLAY_MSGTYPE_BLIT 1
struct OverlayMsgBlit {
	unsigned int x, y, w, h;
};

#define OVERLAY_MSGTYPE_ACTIVE 2
struct OverlayMsgActive {
	unsigned int x, y, w, h;
};

#define OVERLAY_MSGTYPE_PID 3
struct OverlayMsgPid {
	unsigned int pid;
};

#define OVERLAY_MSGTYPE_FPS 4
struct OverlayMsgFps {
	float fps;
};
#define OVERLAY_FPS_INTERVAL 0.25f

#define OVERLAY_MSGTYPE_INTERACTIVE 5
struct OverlayMsgInteractive {
	bool state;
};

struct OverlayMsg {
	union {
		char headerbuffer[1];
		struct OverlayMsgHeader omh;
	};
	union {
		char msgbuffer[1];
//		struct OverlayMsgShmem oms;
		struct OverlayMsgInit omi;
		struct OverlayMsgBlit omb;
		struct OverlayMsgActive oma;
		struct OverlayMsgPid omp;
		struct OverlayMsgFps omf;
		struct OverlayMsgInteractive omin;
	};
};

#endif
