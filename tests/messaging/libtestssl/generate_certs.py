#!/usr/bin/env python3

"""Certificates and keys generation script.

This script generates certificates and keys with this structure:
                       ╔═══════════╗                             ╔═══════════╗
                       ║ RootCA_A  ║                             ║ RootCA_B  ║
                       ╚═════╤═════╝                             ╚═════╤═════╝
              ┌──────────────┴─────────────┐                    ┌──────┴──────┐
              ▼                            ▼                    ▼             ▼
          ╔═══════╗                    ╔═══════╗          ╔══════════╗   ╔══════════╗
          ║ CA_A1 ║                    ║ CA_A2 ║          ║ Server B ║   ║ Client B ║
          ╚═══╤═══╝                    ╚═══╤═══╝          ╚══════════╝   ╚══════════╝
       ┌──────┴──────┐              ┌──────┴──────┐
       ▼             ▼              ▼             ▼
 ╔═══════════╗ ╔═══════════╗  ╔═══════════╗ ╔═══════════╗
 ║ Server A1 ║ ║ Client A1 ║  ║ Server A2 ║ ║ Client A2 ║
 ╚═══════════╝ ╚═══════════╝  ╚═══════════╝ ╚═══════════╝

It also can output C++ variables declaration with the content of the certificates chains and their keys
by passing the '--print-cxx' option on the command line.

The generation of files can be disabled by passing the '--no-files' option on the command line.
"""

from cryptography import x509
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives.serialization import Encoding, PrivateFormat, NoEncryption
from cryptography.x509.oid import NameOID
import datetime
import ipaddress
import argparse

ONE_DAY = datetime.timedelta(1)
DURATION = 10000 * ONE_DAY # almost 30 years.

def subject_name(name):
    return x509.Name([
        x509.NameAttribute(NameOID.COMMON_NAME, u"libqi testssl {}".format(name)),
        x509.NameAttribute(NameOID.COUNTRY_NAME, "FR"),
        x509.NameAttribute(NameOID.STATE_OR_PROVINCE_NAME, "Paris"),
        x509.NameAttribute(NameOID.LOCALITY_NAME, "Paris"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "Aldebaran"),
        x509.NameAttribute(NameOID.ORGANIZATIONAL_UNIT_NAME, "Framework Team"),
      ])

class EndEntity(object):
    @classmethod
    def extensions(cls, builder, pub_key, issuer):
        if issuer is not None:
            builder = builder.add_extension(
                    x509.AuthorityKeyIdentifier.from_issuer_public_key(issuer.public_key()),
                    critical=False)
        builder = builder.add_extension(
                x509.BasicConstraints(ca=False, path_length=None),
                critical=False)
        builder = builder.add_extension(
                x509.KeyUsage(
                    digital_signature=True,
                    content_commitment=False,
                    key_encipherment=True,
                    data_encipherment=True,
                    key_agreement=True,
                    key_cert_sign=False,
                    crl_sign=False,
                    encipher_only=False,
                    decipher_only=False),
                critical=True)
        builder = builder.add_extension(
                x509.SubjectKeyIdentifier.from_public_key(pub_key),
                critical=False)
        return builder

class CertificateAuthority(object):
    @classmethod
    def extensions(cls, builder, pub_key, issuer):
        if issuer is not None:
            builder = builder.add_extension(
                x509.AuthorityKeyIdentifier.from_issuer_public_key(issuer.public_key()),
                critical=False)
        builder = builder.add_extension(
                x509.BasicConstraints(ca=True, path_length=None),
                critical=True)
        builder = builder.add_extension(
                x509.KeyUsage(
                    digital_signature=False,
                    content_commitment=False,
                    key_encipherment=False,
                    data_encipherment=False,
                    key_agreement=False,
                    key_cert_sign=True,
                    crl_sign=True,
                    encipher_only=False,
                    decipher_only=False),
                critical=True)
        builder = builder.add_extension(
                x509.SubjectKeyIdentifier.from_public_key(pub_key),
                critical=False)
        builder = builder.add_extension(
                x509.NameConstraints(
                    permitted_subtrees = [
                        x509.IPAddress(ipaddress.IPv4Network("127.0.0.0/8")),
                        x509.IPAddress(ipaddress.IPv6Network("::1")),
                        x509.DNSName("localhost")],
                    excluded_subtrees = None),
                critical=True)
        return builder

class Server(EndEntity):
    @classmethod
    def extensions(cls, builder, pub_key, issuer):
        builder = super().extensions(builder, pub_key, issuer)
        builder = builder.add_extension(
                x509.SubjectAlternativeName([
                    x509.IPAddress(ipaddress.IPv4Address("127.0.0.1")),
                    x509.IPAddress(ipaddress.IPv6Address("::1")),
                    x509.DNSName("localhost"),
                    ]),
                critical=False)
        builder = builder.add_extension(
                x509.ExtendedKeyUsage([
                    x509.ExtendedKeyUsageOID.SERVER_AUTH
                    ]),
                critical=True)
        return builder

class Client(EndEntity):
    @classmethod
    def extensions(cls, builder, pub_key, issuer):
        builder = super().extensions(builder, pub_key, issuer)
        builder = builder.add_extension(
                x509.ExtendedKeyUsage([
                    x509.ExtendedKeyUsageOID.CLIENT_AUTH
                    ]),
                critical=True)
        return builder

class Cert(object):
    def __init__(self, name, issuer, type):
        self.name = name
        self.type = type
        self.private_key = rsa.generate_private_key(
            public_exponent=65537,
            key_size=2048)
        self.public_key = self.private_key.public_key()
        subject = subject_name(name)
        if issuer is None:
            (issuer, issuer_name, issuer_key) = (None, subject, self.private_key)
        else:
            (issuer, issuer_name, issuer_key) = (issuer.cert, issuer.cert.subject, issuer.private_key)
        builder = x509.CertificateBuilder()
        builder = builder.subject_name(subject)
        builder = builder.issuer_name(issuer_name)
        builder = builder.not_valid_before(datetime.datetime.today())
        builder = builder.not_valid_after(datetime.datetime.today() + DURATION)
        builder = builder.serial_number(x509.random_serial_number())
        builder = builder.public_key(self.public_key)
        builder = self.type.extensions(builder, self.public_key, issuer)
        self.cert = builder.sign(private_key=issuer_key, algorithm=hashes.SHA256())

def bytes_to_cxx_string(bytes):
    return "\n" + "\n".join(["\"{}\\n\"".format(l) for l in bytes.decode("utf-8").splitlines()])

def indent():
    return "  "

def decl_cxx_string(name, value):
    return "const std::string {} ={};".format(name, ("\n" + indent()).join(value.splitlines()))

def cert_cxx_suffix(prefix):
    return prefix + "Cert"

def certschain_cxx_suffix(prefix):
    return prefix + "CertsChain"

def key_cxx_suffix(prefix):
    return prefix + "Key"

def variable_name(text):
    # remove spaces and lower the first letter of the variable.
    text = text.replace(" ", "")
    if len(text) > 0:
        text = text[0].lower() + text[1:]
    return text

class Chain(object):
    def __init__(self, chain = []):
        self.chain = chain

    def add(self, name, type):
        chain = []
        chain.extend(self.chain)
        issuer = self.end_entity()
        chain.insert(0, Cert(name, issuer, type))
        return Chain(chain)

    def end_entity(self):
        if len(self.chain) == 0:
            return None
        return self.chain[0]

    def ca(self):
      if len(self.chain) < 2:
          return None
      return self.chain[1]

    def root(self):
        if len(self.chain) == 0:
            return None
        return self.chain[-1]

    def public_bytes(self):
        return self.end_entity().cert.public_bytes(Encoding.PEM)

    def key_private_bytes(self):
        return self.end_entity().private_key.private_bytes(
                Encoding.PEM,
                format=PrivateFormat.PKCS8,
                encryption_algorithm=NoEncryption())

    def cxx_code(self):
        cxx_name = variable_name(self.end_entity().name)
        key_code = decl_cxx_string(key_cxx_suffix(cxx_name), bytes_to_cxx_string(self.key_private_bytes()))
        cert_name = cert_cxx_suffix(cxx_name)
        cert_code = decl_cxx_string(cert_name, bytes_to_cxx_string(self.public_bytes()))
        certs_chain_value = " " + cert_name
        ca = self.ca()
        if ca is not None:
          certs_chain_value += " + " + certschain_cxx_suffix(variable_name(ca.name))
        certs_chain_code = decl_cxx_string(certschain_cxx_suffix(cxx_name), certs_chain_value)
        return "{}\n{}\n{}".format(key_code, cert_code, certs_chain_code)

    def write_files(self):
        filename_prefix = self.end_entity().name.replace(" ", "_")
        with open(filename_prefix + ".key", mode="xb") as file:
            file.write(self.key_private_bytes())
        with open(filename_prefix + ".crt", mode="xb") as file:
            file.write(self.public_bytes())

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Generate certificates")
    parser.add_argument("--print-cxx", action="store_true")
    parser.add_argument("--no-files", action="store_true")
    args = parser.parse_args()

    root_ca_a = Chain().add("Root CA A", CertificateAuthority)
    ca_a1 = root_ca_a.add("CA A1", CertificateAuthority)
    server_a1 = ca_a1.add("Server A1", Server)
    client_a1 = ca_a1.add("Client A1", Client)

    ca_a2 = root_ca_a.add("CA A2", CertificateAuthority)
    server_a2 = ca_a2.add("Server A2", Server)
    client_a2 = ca_a2.add("Client A2", Client)

    root_ca_b = Chain().add("Root CA B", CertificateAuthority)
    server_b = root_ca_b.add("Server B", Server)
    client_b = root_ca_b.add("Client B", Client)

    for chain in [root_ca_a,
                  ca_a1,
                  server_a1,
                  client_a1,
                  ca_a2,
                  server_a2,
                  client_a2,
                  root_ca_b,
                  server_b,
                  client_b]:
        if not args.no_files:
            chain.write_files()
        if args.print_cxx:
            print(chain.cxx_code())

