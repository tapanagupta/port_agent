#!/usr/bin/env python

from fabric.api import *
from fabric.contrib.console import confirm
import os
import re

# What does this release script do:
# get major.minor.micro from setup.py
# Chop off -dev tag at the end. If no dev at the end, bump the micro
# version
# commit it.
# Tag at the commit.
# Bump to the next dev version: major.minor.micro+1'-dev'
# commit it.
# Push changes and tags to remote

# How do you use a release like this:
# Get it from the GitHub by using the tag + tar ball feature

versionTemplates = {
        'release': '#define PORT_AGENT_VERSION "%(major)s.%(minor)s.%(micro)s"'
        , 'dev': '#define PORT_AGENT_VERSION "%(major)s.%(minor)s.%(micro)s-dev"'
        , 'git-tag': 'v%(major)s.%(minor)s.%(micro)s'
        , 'git-message': 'Release Version %(major)s.%(minor)s.%(micro)s'
        , 'dev-message': 'Bump Version to %(major)s.%(minor)s.%(micro)s-dev'
        }

# Monkey-patch "open" to honor fabric's current directory
_old_open = open
def open(path, *args, **kwargs):
    return _old_open(os.path.join(env.lcwd, path), *args, **kwargs)

def _validateVersion(v):
    versionRe = re.compile('^(?P<major>[0-9]+)\\.(?P<minor>[0-9]+)\\.(?P<micro>[0-9]+)(?P<pre>[-0-9a-zA-Z]+)?$')
    m = versionRe.match(v)
    if not m:
        raise Exception('Version must be in the format <number>.<number>.<number>[<string>]')

    valDict = m.groupdict()
    for k in ('major', 'minor', 'micro'): valDict[k] = int(valDict[k])
    return valDict

# Decorator class to fab target
class _cloneDir(object):
    def __init__(self, gitUrl, project, default_branch):
        self.gitUrl = gitUrl
        self.project = project
        self.default_branch = default_branch

    def __call__(self, f):
        def wrapped_f(*args, **kwargs):
            local('rm -rf ../tmpfab')
            local('mkdir ../tmpfab')
            local('git clone %s ../tmpfab/%s' % (self.gitUrl, self.project))

            remote = 'fab'
            with lcd(os.path.join('..', 'tmpfab', self.project)):
                branch = prompt('Please enter release branch:',
                    default=self.default_branch)
                local('git remote add %s %s' % (remote, self.gitUrl))
                local('git fetch %s' % remote)

                local('git checkout -b fab_%s %s/%s' % (branch, remote, branch))
                kwargs['branch'] = branch
                kwargs['remote'] = remote
                f(*args, **kwargs)
            local('rm -rf ../tmpfab')
        return wrapped_f

def _getReleaseVersion():
    # Get current python version
    currentVersionStr = local('grep PORT_AGENT_VERSION src/version.h | sed ' + "'" +'s/#define PORT_AGENT_VERSION "\(.*\)"/' + "\\" + '1/' + "'", capture=True).strip()
    cvd = _validateVersion(currentVersionStr)
    if not currentVersionStr.endswith('-dev'):
        cvd['micro'] += 1

    nextVersionStr = '%d.%d.%d' % (cvd['major'], cvd['minor'], cvd['micro'])
    print 'You current version is %s.  You release version will be %s.' % (currentVersionStr, nextVersionStr)

    return cvd

def _replaceVersionInFile(filename, matchRe, template, versionCb):
    with open(filename, 'r') as rfile:
        lines = rfile.readlines()

    currentVersionStr = None
    for linenum,line in enumerate(lines):
        m = matchRe.search(line)
        if m:
            vals = m.groupdict()
            indent, currentVersionStr, linesep = vals['indent'], vals['version'], line[-1]
            break

    if currentVersionStr is None:
        abort('Version not found in %s.' % (filename))
    version = versionCb(currentVersionStr)
    nextVersionStr = '%s%s%s' % (indent, template % version, linesep)

    lines[linenum] = nextVersionStr
    with open(filename, 'w') as wfile:
        wfile.writelines(lines)

def _gitTag(version):
    versionTag = versionTemplates['git-tag'] % version
    versionMsg = versionTemplates['git-message'] % version

    comment = prompt('Optional comment for this release:', default='')
    if comment != '':
        versionMsg += ': ' + comment
    local('git commit -am "%s"' % (versionMsg))
    commit = local('git rev-parse --short HEAD', capture=True)
    local('git tag -af %s -m "%s" %s' % (versionTag, versionMsg, commit))

    print versionTag, versionMsg, commit

@_cloneDir(gitUrl='git@github.com:ooici/port_agent.git',
    project='port_agent',
    default_branch='master')
def release(branch, remote='fab'):

    # Deduce release version
    nextVersionD = _getReleaseVersion()

    # Update setup.py to release version
    version_re = re.compile('(?P<indent>\s*)#define PORT_AGENT_VERSION "(?P<version>[^\s]+)"')
    _replaceVersionInFile('src/version.h', version_re,
            versionTemplates['release'], lambda old: nextVersionD)
    # Tag at release version
    _gitTag(nextVersionD)

    # Immediately go to next dev version to ensure release version is tied
    # to one commit only
    nextVersionD['micro'] += 1
    _replaceVersionInFile('src/version.h', version_re,
            versionTemplates['dev'], lambda old: nextVersionD)

    devMsg = versionTemplates['dev-message'] % nextVersionD
    local('git commit -am "%s"' % devMsg)

    remote = 'origin'

    # Push commits and tags
    local('git push %s --tags' % (remote))
    local('git push %s HEAD:%s' % (remote, branch))

host = None
env.user = 'buildbot-runner'

def ion_alpha():
    global host
    host = 'rsn-port-agent-test.oceanobservatories.org'
    env.host_string = host

def ion_beta():
    global host
    host = 'pl-port-agent01.oceanobservatories.org'
    env.host_string = host

def deploy():
    global host
    host = host or prompt('Please enter port agent host name: ', default='rsn-port-agent-test.oceanobservatories.org')
    env.host_string = host
    print env

    with cd('~/port_agent'):
        run("rm -rf clone")
        # If port_agent dir exist, take a short cut on cloning to save network bandwidth.
        run("(if [ -r port_agent ]; then cd port_agent; git clone . ../clone; cd ../clone; git remote add fab git://github.com/ooici/port_agent.git; git fetch fab; else git clone git://github.com/ooici/port_agent.git clone;fi)")
        run("rm -rf port_agent")
        run("(cd clone; git clone . ../port_agent)")
    code_dir = '~/port_agent/port_agent'
    with cd(code_dir):
        run("echo PORT AGENT VERSIONS")
        cmd = "git tag | sed -e 's/v//g' | sort -t. -k1,1n -k2,2n -k3,3n | sed -e 's/^/v/g'"
        run(cmd)
        currentVersionStr = run(cmd + ' | tail -n1').strip()
        versionStr = prompt('Please enter version to be deployed:',
               default=currentVersionStr)
        run("git checkout %s" % versionStr)
        run('./configure --prefix=/opt/ooi/port_agent.%s' % versionStr[1:])
        with settings(warn_only=True):
            result = run('make check')
            if result.failed and not confirm("Tests failed.  Continue anyway?"):
                abort("Aborting at user request.")
        run("make install")
        if confirm("Soft link /opt/ooi/port_agent.%s to \
                /opt/ooi/port_agent?" % versionStr[1:]):
            run('echo Linking now')
            run("rm /opt/ooi/port_agent")
            run("ln -s /opt/ooi/port_agent.%s/ /opt/ooi/port_agent" % versionStr[1:])
