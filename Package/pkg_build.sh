#!/bin/sh

#  pkg_build.sh
#  get-manip
#
#  Created by Joshua Bradt on 12/30/14.
#  Copyright (c) 2014 NSCL. All rights reserved.


VERSION=`cat ./Package/version`

PACKAGE_NAME=get-manip

pkgbuild --root "${INSTALL_ROOT}" \
--identifier "edu.msu.nscl.attpc.get-manip" \
--version "$VERSION" \
--install-location "/" \
"${BUILT_PRODUCTS_DIR}/get-manip.pkg"

productbuild --distribution "./Package/Distribution.xml"  \
--package-path "${BUILT_PRODUCTS_DIR}" \
"${BUILT_PRODUCTS_DIR}/get-manip-${VERSION}.pkg"