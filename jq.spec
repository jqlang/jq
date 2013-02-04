# This is spec file maintained by developers of JQ, not by a OS distro.
# Your OS of choice will likely ignore this RPM spec file.
Summary: Command-line JSON processor
Name: jq
Version: %{myver}
Release: %{myrel}%{?dist}
Source0: jq-%{myver}.tar.gz
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

# Crank up the compression
%define _binary_payload w7.lzdio

%description
jq is a command-line JSON processor

%prep
%setup

%build
echo "Building in: \"$(pwd)\""
%if "%{devbuild}" == "yes"
./configure --prefix=%{_prefix} --enable-devel
%else
./configure --prefix=%{_prefix}
%endif
make

%install
echo "Installing to: \"%{buildroot}\""
make install DESTDIR=%{buildroot}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_bindir}/jq
%if "%{devbuild}" == "yes"
%{_libexecdir}/%{name}/jq_test
%{_libexecdir}/%{name}/testdata
%endif
%{_datadir}/doc/%{name}/AUTHORS
%{_datadir}/doc/%{name}/COPYING
%{_datadir}/doc/%{name}/INSTALL
%{_datadir}/doc/%{name}/NEWS
%{_datadir}/doc/%{name}/README
%{_datadir}/doc/%{name}/README.md
%{_datadir}/man/man1/jq.1

%changelog

%pre

%post
