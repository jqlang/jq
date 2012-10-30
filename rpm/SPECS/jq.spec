Summary: Command-line JSON processor
Name: jq
Version: %{version}
Release: %{release}
Source0: jq-%{version}.tgz
URL: https://github.com/stedolan/jq
 
License: Copyright (C) 2012 Stephen Dolan
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
 
%install
echo "Building in: \"%{buildroot}\""
rm -rf %{buildroot}
install -d -m 755 %{buildroot}/usr/bin
mv %{_builddir}/jq-%{version}/bin/jq %{buildroot}/usr/bin

%clean

%files
%defattr(-,root,root)
/usr/bin/jq
 
%changelog

%pre

%post
