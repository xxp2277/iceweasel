#!/usr/bin/python
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import json
import os
import sys
import glob
import shutil
import subprocess

import mozcrash
from mozbuild.base import MozbuildObject
from mozfile import TemporaryDirectory
from mozhttpd import MozHttpd
from mozprofile import FirefoxProfile, Preferences
from mozprofile.permissions import ServerLocations
from mozrunner import FirefoxRunner, CLI
from six import string_types
from selenium import webdriver
from selenium.webdriver.firefox.firefox_binary import FirefoxBinary
from selenium.webdriver.firefox.firefox_profile import FirefoxProfile

PORT = 8888

PATH_MAPPINGS = {
    '/webkit/PerformanceTests': 'third_party/webkit/PerformanceTests',
    # It is tempting to map to `testing/talos/talos/tests` instead, to avoid
    # writing `tests/` in every path, but we can't do that because some files
    # refer to scripts located in `../..`.
    '/talos': 'testing/talos/talos',
}

if __name__ == '__main__':
    cli = CLI()
    debug_args, interactive = cli.debugger_arguments()
    runner_args = cli.runner_args()

    build = MozbuildObject.from_environment()

    binary = runner_args.get('binary')
    if not binary:
        binary = build.get_binary_path(where="staged-package")
    binary = os.path.normpath(os.path.abspath(binary))

    path_mappings = {
        k: os.path.join(build.topsrcdir, v)
        for k, v in PATH_MAPPINGS.items()
    }
    httpd = MozHttpd(port=PORT,
                     docroot=os.path.join(build.topsrcdir, "build", "pgo"),
                     path_mappings=path_mappings)
    httpd.start(block=False)

    locations = ServerLocations()
    locations.add_host(host='127.0.0.1',
                       port=PORT,
                       options='primary,privileged')

    cmdargs = "http://localhost:%d/index.html" % PORT

    if os.path.exists("test_png"):
        shutil.rmtree("test_png")
    os.mkdir("test_png")
try:
    i = 1;
    fireFoxOptions = webdriver.FirefoxOptions()
    fireFoxOptions.set_headless()
    binary = FirefoxBinary(binary)
    driver = webdriver.Firefox(firefox_binary=binary, firefox_options=fireFoxOptions)
    m_file = os.path.abspath(os.path.dirname(__file__))
    m_list = open(m_file + "\\url.txt") 
    for line in m_list.readlines(): 
        m_png = "test_png\\test_%d.png" % i
        line=line.strip('\n')  
        if line.startswith('http'):
            cmdargs = line
        else:
            cmdargs = "http://localhost:%d/%s" % (PORT, line)
        print(cmdargs);
        driver.get(cmdargs)
        driver.get_screenshot_as_file(m_png)
        i += 1;
    m_list.close()
finally:
    try:
        driver.quit()
        httpd.stop()
    except:
        pass