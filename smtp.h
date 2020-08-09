#ifndef MIMEDOWN_SMTP_H
#define MIMEDOWN_SMTP_H

char *generate_msgid(const char *host);
char *generate_mime_boundary(void);
const char *filename_mime(const char *f);

#endif
