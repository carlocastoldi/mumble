// Copyright 2021-2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_MUMBLE_IPCUTILS_H_
#define MUMBLE_MUMBLE_IPCUTILS_H_

#include <QtCore/QDir>

namespace Mumble {

	/**
	 * @returns The path where Mumble sets up MumbleOverlayPipe and MumbleSocket
	 */
	QDir getRuntimeDir(void);

}; // namespace Mumble

#endif // MUMBLE_MUMBLE_IPCUTILS_H_
