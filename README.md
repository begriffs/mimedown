## Markdown to multipart MIME

Answering the age-old debate: HTML or plaintext emails? Both!

Write email in markdown, and convert to multipart MIME that works impeccably in
graphical and console mail clients.

* Creates a conservative [ISO/IEC 15445:2000
  HTML](https://www.iso.org/standard/27688.html) representation, which is set
  as the preferred multipart alternative for graphical clients.
* The text alternative is reflowed at 72 characters to look good in standard
  console mail readers.
* The text part also uses format=flowed, so it will shrink on narrow screens or
  terminal splits.
* Fenced code blocks in the markdown are added as inline attachments with the
  appropriate MIME type. This makes them easy to download/save from the
  message.
* URLs are listed at the bottom of the text message in a special
  `text/uri-list` inline attachment, and referred to with short codes like
  `[0]`, `[1]` ... in the message text.

### Installation

Written in portable C99. The only requirement is the
[cmark](https://github.com/commonmark/cmark) library to parse markdown.

```sh
# detect cmark and set up build flags
./configure
# then build md2mime
make
```

### Usage

```sh
./md2mime < message.md > message.email
```

Then edit message.email to add headers like Subject, etc. Send the message
using a program like [msmtp](https://marlam.de/msmtp/):

```sh
msmtp recipient@example.com < message.email
```
