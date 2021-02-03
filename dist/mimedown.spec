# run "spectool -g -R mimedown.spec" to automatically download source files
# spectool is part of the rpmdevtools package
%global commit 2f298c3e53ad49e6c2aa0e1cca940ab04fa7f02c
%global shortcommit %(c=%{commit}; echo ${c:0:7})
%global repo begriffs

Name:       mimedown
Version:    2f298c3
Release:    1%{?dist}
Summary:    A tool to display user-oriented Slurm partition information.

Group:      System Environment/Base
License:    GPLv2+
URL:        https://github.com/%{repo}/%{name}
Source:     https://github.com/%{repo}/%{name}/archive/%{commit}/%{name}-%{shortcommit}.tar.gz

BuildRequires:   cmark-devel
Requires:   cmark

%description
%{name} is a tool to convert markdown to multipart MIME that works impeccably in graphical and console mail clients.

%prep
%autosetup -n %{name}-%{commit}

%build
%configure
make %{?_smp_mflags}

%install
mkdir -p %{buildroot}%{_bindir}
install -m 0755 ./md2mime %{buildroot}%{_bindir}/md2mime

%files
%defattr(755,root,root)
%{_bindir}/md2mime


%changelog
* Mon Feb 01 2021 Kilian Cavalotti <kilian@stanford.edu>
- initial package
