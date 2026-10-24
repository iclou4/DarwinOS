# Legal, Trademark and Copyright Compliance

## 1. Non-Affiliation and Trademark Disclaimer

**DarwinOS** is an independent, non-commercial open-source research and educational operating system project.

- **DarwinOS is NOT affiliated with, sponsored by, endorsed by, or in any way associated with Apple Inc.**
- "Apple", "macOS", "OS X", "Mac OS X", "Cocoa", "AppKit", "Metal", "Aqua", "Finder", and other associated names and logos are registered trademarks or trademarks of **Apple Inc.** in the United States and other countries.
- All references to Apple trademarks, brand names, APIs, frameworks, and operating system versions are used strictly for **nominative, descriptive, and compatibility identification purposes** under the Fair Use doctrine (15 U.S.C. § 1115(b)(4)).
- No trademark infringement or endorsement is intended or implied.

---

## 2. Open Source Licenses & Provenance

DarwinOS is built upon freely licensed, publicly released open-source technologies:

1. **Darwin / XNU Kernel and Core OS Components:**
   - Released by Apple Inc. as open source under the **Apple Public Source License (APSL Version 2.0)** and BSD licenses.
   - Available publicly via [Apple Open Source](https://opensource.apple.com/) and [GitHub apple-oss-distributions](https://github.com/apple-oss-distributions).
   - In compliance with Section 2 of APSL 2.0, modifications and derived works preserve Apple copyright headers and grant public source availability.

2. **BSD Subsystems & Libraries:**
   - Derived from FreeBSD / NetBSD under the standard 2-Clause and 3-Clause BSD Licenses.

3. **Cocoa / AppKit Runtime and Desktop Libraries:**
   - Independent clean-room implementations derived from open-source projects including GNUstep, Cocotron, and ravynOS under LGPL 2.1+ / MIT / BSD licenses.

4. **Community Assets and Artwork:**
   - Visual themes and cursors are community-created reconstructions (e.g. macOS Tahoe community cursor pack) intended for educational and non-commercial research use.

---

## 3. Clean-Room Reverse Engineering & Interoperability

All proprietary userland subsystems and display server frameworks implemented in DarwinOS (including, but not limited to, the `SkyLight WindowServer`, `Metal.framework` software rasterizer pipeline, and `AppKit` userland compatibility shims) have been engineered strictly through **clean-room reverse engineering**:

1. **Independent Implementation:** No proprietary Apple source code, copyrighted binary blobs, decompiled proprietary disassembly, or confidential Apple engineering materials were used in the creation of these components.
2. **Purpose of Interoperability:** Reverse engineering, functional analysis, and protocol reconstruction were conducted solely to enable cross-platform interoperability with Darwin/Cocoa software binaries and Mach IPC protocols.
3. **Statutory & Judicial Protection:**
   - **United States Code:** Protected under **17 U.S.C. § 1201(f)**, which explicitly allows reverse engineering of software programs for the sole purpose of achieving interoperability of an independently created computer program with other programs.
   - **Federal Judicial Precedents:** Protected under **Sony Computer Entertainment, Inc. v. Connectix Corp.**, 203 F.3d 596 (9th Cir. 2000) and **Sega Enterprises Ltd. v. Accolade, Inc.**, 977 F.2d 1510 (9th Cir. 1992), establishing that reverse engineering and emulation for interoperability constitute non-infringing Fair Use.
   - **API Copyrightability & Interoperability:** Re-implementation of functional API declarations, Mach message interfaces, and data structures is protected under **Oracle America, Inc. v. Google LLC**, 141 S. Ct. 1183 (2021).
   - **European Union Law:** Protected under **Directive 2009/24/EC**, Articles 5(3) and 6, guaranteeing the right to observe, study, test, and decompile programs without authorization to achieve interoperability.

---

## 4. DMCA and Intellectual Property Policy

DarwinOS strictly respects the intellectual property rights of all copyright holders.
If you believe that any material or contribution in this repository infringes upon your copyright or trademark rights, please contact the repository maintainer directly or submit an issue so that the content can be reviewed and removed immediately.
