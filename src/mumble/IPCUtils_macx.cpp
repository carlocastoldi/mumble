// Copyright 2021-2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "IPCUtils.h"

#include <unistd.h>
//#include <QtCore/QStandardPaths>
#include <QtCore/QDir>

namespace Mumble {
	QDir getRuntimeDir(void) {
		// QString runtimePath = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation); // defaults to "~/Library/Application Support"
		// QDir mumbleRuntimeDir = QDir(runtimePath);
		QDir mumbleRuntimeDir = QDir::home();
		return mumbleRuntimeDir;
	}

}; // namespace Mumble
