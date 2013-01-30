# This is spec file maintained by developers of JQ, not by a OS distro.
# Your OS of choice will likely ignore this RPM spec file.
Summary: Command-line JSON processor
Name: jq
Version: %{version}
Release: %{release}
Source0: jq-%{version}.tar.gz
URL: https://github.com/stedolan/jq
License: BSD
AutoReqProv: no
#BuildPrereq: autoconf, libtool, automake, flex, bison, python

Group: Applications/System
# Requires:

# Disables debug packages and stripping of binaries:
%global _enable_debug_package 0
%global debug_package %{nil}
%global __os_install_post %{nil}

%description
jq is a command-line JSON processor

%prep

%setup

%build
echo "Building in: \"$(pwd)\""
./configure --prefix=%{_prefix}
make

%install
echo "Installing to: \"${_buildroot}\""
make install DESTDIR=${_buildroot}

%clean
rm -rf ${_buildroot}

%files
%defattr(-,root,root)
%{_bindir}/jq
%{_bindir}/jq_test
%{_datadir}/doc/jq/AUTHORS
%{_datadir}/doc/jq/COPYING
%{_datadir}/doc/jq/INSTALL
%{_datadir}/doc/jq/NEWS
%{_datadir}/doc/jq/README
%{_datadir}/doc/jq/README.md

%changelog

%pre

%post
