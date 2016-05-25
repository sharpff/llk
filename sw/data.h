#ifndef __DATA_H__
#define __DATA_H__

extern uint8_t terminalAES[];

const uint8_t *getPreSharedIV();
const uint8_t *getPreSharedToken();
const uint8_t *getTerminalToken();
void getTerminalTokenStr(char *token, int len);
int getPreSharedPublicKey(uint8_t *pubkey, int pubkeyLen);

int getTerminalUUID(uint8_t *uuid, int len);
void setTerminalUUID(const uint8_t *uuid, int len);
int getTerminalPublicKey(uint8_t *pubkey, int pubkeyLen);
void setTerminalPublicKey(const uint8_t *pubkey, int pubkeyLen);
int getTerminalSignature(uint8_t *signature, int len);
void setTerminalSignature(const uint8_t *signature, int len);

void setOriRemoteServer(const char *ip, int len, uint16_t port);
void getOriRemoteServer(char *ip, int len, uint16_t *port);

uint16_t getProtocolVer();
int getVer(char *fwVer, int size);
void setTerminalStatus(const char *status, int len);
int getTerminalStatus(char *status, int len);
void cacheSetTerminalStatus(const char *status, int len);
int cacheGetTerminalStatus(char *status, int len);

int setLock(int locked);
int getLock();

// test only
#define PUBLIC_KEY_PEM \
"-----BEGIN PUBLIC KEY-----\r\n" \
"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDjeG5m0ko6IuRnHSI9szXIGBzY\r\n" \
"Vff9KOjRqSvVnFDUYsERbaO7fRrcZQ4iskGGT7PkTOZUwtf7Hi+dPHVRnEOB6QQ4\r\n" \
"er2U9SkXxAKIEsv6qw47sfbGAc5rvB+H5vSxh96zvXmnNpnwv8vOu2bWqKjABRVx\r\n" \
"+AzRYrUP3vGSK6A2EQIDAQAB\r\n" \
"-----END PUBLIC KEY-----\r\n"

#define PRIATE_KEY_PEM \
"-----BEGIN RSA PRIVATE KEY-----\r\n" \
"MIICXwIBAAKBgQDjeG5m0ko6IuRnHSI9szXIGBzYVff9KOjRqSvVnFDUYsERbaO7\r\n" \
"fRrcZQ4iskGGT7PkTOZUwtf7Hi+dPHVRnEOB6QQ4er2U9SkXxAKIEsv6qw47sfbG\r\n" \
"Ac5rvB+H5vSxh96zvXmnNpnwv8vOu2bWqKjABRVx+AzRYrUP3vGSK6A2EQIDAQAB\r\n" \
"AoGBANXI6Cnm1jBRfsySiw3mElPepa2FDq919WqnTjVS1nzl24KfwtSBPe+JYoGl\r\n" \
"ewPagL2+sHNVNFwlAKcMnU54SNBXqn0UMO40ETQMJ4aPb5pUIKmX9yonvCHBIwHO\r\n" \
"Cxio5eakZVsLLVmhObR+WQ73GabQXsPI/bTj21+70KbH/MhJAkEA/TNimLPSkFaZ\r\n" \
"gmle8Ss1oWB5otqfEMhkPdmMuuEVaWJoGRrlT9NV06XOrjOsoHA//VuL+3HS3jGh\r\n" \
"PF0v65ZPAwJBAOX8OT9rshBeplVd/2YMKOnhGwIqDNQFIjZxIJ2YvWkhIlxoqxK4\r\n" \
"62Bxa7Cb2snVNSrwjWsH4U8wCNoWwSjbYFsCQQCMT1Za4oNTwvmMYBHhuR0eEVU2\r\n" \
"Xsn4xeNutsiaorJ31LV0/AeI5cbQ4zgWJsKJocqD/qAitZ+xy3Ta+5Tbi067AkEA\r\n" \
"p6PZquP6sLn9br6Muzrj289NG5/BKA7x6FM/3gCHGImhfVCCWqxWTj9+qwaATZzP\r\n" \
"G5Sq0Li2wD0YRrhNZlW6fwJBAOiuuyaKCeJnCggAKu0mT35BeHRc7aE802qvuxsi\r\n" \
"xDCHrBhB/xnnmHvpaheMDoEK5OMAqtQckGDlWCekXq8oaCE=\r\n" \
"-----END RSA PRIVATE KEY-----\r\n"

/* TODO: Debug only */
// global
#define PUBLIC_KEY_GLOBAL_PEM \
"-----BEGIN PUBLIC KEY-----\r\n" \
"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCR/q8O94QsX9ylX9L1aG1tccy6wLOYYLpjBv9e\r\n" \
"WMd1TwtJBlcfZimeV+f0/7WtkODnDG9fdl8zcly3J/8i5B5UF7qzWA5Q74g/4TcYN97veYj+w8RN\r\n" \
"OmE6yZt1lbElKrcy1im7H6SIpuEhp1U3UIVnDQKcF/RLEqywshOIzz4+dQIDAQAB\r\n" \
"-----END PUBLIC KEY-----\r\n"

#define PRIATE_KEY_GLOBAL_PEM \
"-----BEGIN PRIVATE KEY-----\r\n" \
"MIICdwIBADANBgkqhkiG9w0BAQEFAASCAmEwggJdAgEAAoGBAJH+rw73hCxf3KVf0vVobW1xzLrA\r\n" \
"s5hgumMG/15Yx3VPC0kGVx9mKZ5X5/T/ta2Q4OcMb192XzNyXLcn/yLkHlQXurNYDlDviD/hNxg3\r\n" \
"3u95iP7DxE06YTrJm3WVsSUqtzLWKbsfpIim4SGnVTdQhWcNApwX9EsSrLCyE4jPPj51AgMBAAEC\r\n" \
"gYEAgnWGcyZlGs4SukiVl6xYVHtjieJW2ixdgc4n9mP44G/g0sFyquE9AXzmDv1NmKqk/nITN8fs\r\n" \
"LGneReDc0Y3tNrddsDc0fQJuZwRVcTnlQnsnJoKbNWZnnDQbn4R2DgaEwzAv1gft9DvhztOpTvR8\r\n" \
"Wd0obxZ0eGTUFJUe9cV1nUUCQQDRXT7oxid4eVckJRzSEK6GK06WRgwaGX6pXjKZAu49+DpWbKQY\r\n" \
"td/Byj9RWMKGggin++0UKstVaDLbgY6xqhSfAkEAsoPfRo0Dq67pfvbdMKpQqxLyuVfNyzanPsPo\r\n" \
"8yKl4okQeeKhV6LqnViv/2OPkImD+RZQhz+8eCZe/1FTNxRgawJBAKMp+3Qnse0nC62kqsBftdPk\r\n" \
"fluI/bd9dQQD4IT1Dr0G7lgaVQzY+SgrOYtpFDkFsD9Si+Jys4K9MR1268E0XtsCQBsUskxrnADx\r\n" \
"ee6O2KNHUxGAwF2pmkiXs+troO0xdmTVUjCLVtqKlnjkOV7O5bqRgfnJjtHVs2uutn6ZAby7VJ0C\r\n" \
"QCi8rnrRk2iyibs+7PZAFCjxB34C78n5EyDCL92JfFOziRXZLl5lvkRruO7Lcal7eh5fHAwr3YG9\r\n" \
"ZwrOpUNhayI=\r\n" \
"-----END PRIVATE KEY-----\r\n"

/* TODO: Debug only */
// A kind of DEV
#define PUBLIC_KEY_DEVKIND_PEM \
"-----BEGIN PUBLIC KEY-----\r\n" \
"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDCyQMwvQauI1/PbtQ2FwVTZwDcPcDXI1nEUIvq\r\n" \
"sz+tlmQzwpCWGUOwHbZF3AVw8b1zvF5nW/UU0aF8z2KKCqtm6gB4jSblbJZDUlvMhASiGnCUGg2l\r\n" \
"Hf3MDtiMFOeGy2XmvBLDLZVP3uU8gDLfTfCqW+JWzTqoEBZrEK5IPQbi+wIDAQAB\r\n" \
"-----END PUBLIC KEY-----\r\n"

#define PRIATE_KEY_DEVKIND_PEM \
"-----BEGIN PRIVATE KEY-----\r\n" \
"MIICeAIBADANBgkqhkiG9w0BAQEFAASCAmIwggJeAgEAAoGBAMLJAzC9Bq4jX89u1DYXBVNnANw9\r\n" \
"wNcjWcRQi+qzP62WZDPCkJYZQ7AdtkXcBXDxvXO8Xmdb9RTRoXzPYooKq2bqAHiNJuVslkNSW8yE\r\n" \
"BKIacJQaDaUd/cwO2IwU54bLZea8EsMtlU/e5TyAMt9N8Kpb4lbNOqgQFmsQrkg9BuL7AgMBAAEC\r\n" \
"gYEAsQwm0zkthkDvCEvTpeqc06cvWADldGdUY6VW2Bjfi3fEUmvKIjSKmt0W3T7Uz0lbi4QvRsGF\r\n" \
"7ctxflG+Xny+N28ZHGHm3jnEa+xabEnx1O05fYdzUeJ5AZKTgpHAifnsGsCMP200CBg3+p7EAhC9\r\n" \
"I62fWDYH8Wrynm5/XExbnxkCQQD78WcNIEoT7GL3rerIFIvP84AJ1WxKoOv3ZP7XKIeduer1mJH5\r\n" \
"4dEoGu2QGL7F+GlCYrD99AMT9lbgZsPIdCcPAkEAxev8SZMu2i8sp1ZTRgNNSmWEGs4bEgReDMe9\r\n" \
"Zw7OudpTQwLn0uj45W93uzE/oRM4NGjiZQ2Vijxx/Y1YPCNlVQJBAPBZPp82E5VoKI3yW42elDbB\r\n" \
"LH+1HAch95pevgMShjYBHFZJw9DoDbm93q5d/Pkt33TWY9URSTJtCjovr0z3Ch8CQFeq6MwaMqWY\r\n" \
"Qbo5ZZdcfQ7YGoTZCGqZnqMgkkek6nKuPzUug0sLwLu3/Rvge6O4ZKvqTWWfd76MeZ1qz0uMDiUC\r\n" \
"QQDpNarGbcwGkKRdt3bc1y/R+LUebDzsMFkr/BW2SjbJUgsg0oiQ89AaPiPHGTOowxi5TBvg7DWN\r\n" \
"qqlRKvGwemQE\r\n" \
"-----END PRIVATE KEY-----\r\n"

/* TODO: Debug only */
extern const uint8_t ginStrPubkeyPem[];
extern const int ginStrPubkeyPemLen;
extern const uint8_t ginStrPrikeyPem[];
extern const int ginStrPrikeyPemLen;

/* built-in global rsa pubkey */
extern const uint8_t ginPubkeyGlobalDer[];
extern const int ginPubkeyGlobalDerLen;
/* TODO: Debug only */
extern const uint8_t ginPubkeyDevKindDer[];
extern const int ginPubkeyDevKindDerLen;
extern const uint8_t ginPubkeyDEV1Der[];
extern const int ginPubkeyDEV1DerLen;
extern const uint8_t ginPubkeyDEV2Der[];
extern const int ginPubkeyDEV2DerLen;
extern const uint8_t ginPubkeySDK1Der[];
extern const int ginPubkeySDK1DerLen;

/* TODO: Debug only */
extern const uint8_t ginPrikeyGlobalDer[];
extern const int ginPrikeyGlobalDerLen;
extern const uint8_t ginPrikeyDevKindDer[];
extern const int ginPrikeyDevKindDerLen;


#define PRIATE_KEY_TEST_VERIFY_PEM \
"-----BEGIN PUBLIC KEY-----\r\n" \
"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC3rJx6cz+q89VAqnQpx9Cq5TkK\r\n" \
"NOmwu3xHFMG6P/o++BFjOO1cJiq70MdzS8U+SJlKmnaAApY5fsBcet6jafb+h7qn\r\n" \
"QSXVZXr3zhm5HB8h463hmMLzCCLQ7VoGj921iC9IwDWqkuBJrEQkuG2gmInT7nE3\r\n" \
"0oZaJjp3xn8k8rmfHQIDAQAB\r\n" \
"-----END PUBLIC KEY-----\r\n"

#endif
