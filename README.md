# Documentation

Please see the individual manual pages [wcjson(1)](wcjson.1.md), [wcjson(3)](wcjson.3.md) and [wcjson-document(3)](wcjson-document.3.md).

# Releases

Releases are provided via Github. The latest release is
[v0.7](https://github.com/wcjson/wcjson/releases/tag/v0.7).

v0.x releases are marked pre releases not ready for production. Although the
API is quite stable and this is used in production, library version-info will
stick at 0.0.0 until the first stable v1.0 release. The best way to get to that
first stable release is to provide feedback to the project. Especially porting
to operating systems other than Debian/GNU Linux or OpenBSD has not received any
testing so far. For example, if the configure script does not work
for you on your platform, chances are great you are on your way to create your
first pull request providing the changes needed to make it work for you there.
For the use cases this library has been written for - namely not forcing
the heap on users, full support for unicode and wide strings - things are
working quite well so far. That does not mean the API may not change in the
v0.x range of releases in incompatible ways if things do not fit a usecase
I did not think about when writing it.
