# Contributing code to libolm

To contribute code to this library, the preferred way is to clone the git
repository, create a git patch series (for example via ``git
format-patch --stdout origin/master``), and send this by email to
``olm@matrix.org``.

Naturally, you must be willing to license your contributions under the same
license as the project itself - in this case, Apache Software License v2 (see
[LICENSE](LICENSE)).

## Sign off

In order to have a concrete record that your contribution is intentional and
you agree to license it under the same terms as the project's license, we've
adopted the same lightweight approach that the
[Linux Kernel](https://www.kernel.org/doc/html/latest/process/submitting-patches.html#sign-your-work-the-developer-s-certificate-of-origin),
[Docker](https://github.com/docker/docker/blob/master/CONTRIBUTING.md),
and many other projects use: the DCO ([Developer Certificate of Origin](http://developercertificate.org/)).
This is a simple declaration that you wrote the contribution or otherwise have
the right to contribute it to Matrix::

    Developer Certificate of Origin
    Version 1.1

    Copyright (C) 2004, 2006 The Linux Foundation and its contributors.
    660 York Street, Suite 102,
    San Francisco, CA 94110 USA

    Everyone is permitted to copy and distribute verbatim copies of this
    license document, but changing it is not allowed.

    Developer's Certificate of Origin 1.1

    By making a contribution to this project, I certify that:

    (a) The contribution was created in whole or in part by me and I
        have the right to submit it under the open source license
        indicated in the file; or

    (b) The contribution is based upon previous work that, to the best
        of my knowledge, is covered under an appropriate open source
        license and I have the right under that license to submit that
        work with modifications, whether created in whole or in part
        by me, under the same open source license (unless I am
        permitted to submit under a different license), as indicated
        in the file; or

    (c) The contribution was provided directly to me by some other
        person who certified (a), (b) or (c) and I have not modified
        it.

    (d) I understand and agree that this project and the contribution
        are public and that a record of the contribution (including all
        personal information I submit with it, including my sign-off) is
        maintained indefinitely and may be redistributed consistent with
        this project or the open source license(s) involved.

If you agree to this for your contribution, then all that's needed is to
include the line in your commits or covering email::

    Signed-off-by: Your Name <your@email.example.org>

...using your real name; unfortunately pseudonyms and anonymous contributions
can't be accepted. Git makes this trivial - just use the -s flag when you do
``git commit``, having first set ``user.name`` and ``user.email`` git configs
(which you should have done anyway :)
