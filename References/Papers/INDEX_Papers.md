# Papers — Annotated Bibliography

Research papers and surveys found for CyberUIW, recommended by the professor's
reading list (IEEE, ASEE, ASME). Added Sep 30, 2026.

**Read this before citing anything here.** These entries came from web-search
result titles and links only. Nobody has opened the full papers yet, so:

- **Authors, years, venues and page numbers are NOT recorded here on purpose.**
  Open each link and copy the exact citation from the paper itself.
- A "What it may help with" line is a guess from the title. Confirm it after
  reading, then edit the entry and change the status.

**Status key:**
- **To read**: found, not yet opened
- **Read: useful**: opened, will cite (fill in the citation below)
- **Read: not useful**: opened and rejected (say why, so nobody re-evaluates it)

**Copyright rule (same as `Books/`):** paywalled IEEE and ASME papers stay as
links only. Save a PDF in this folder only if the paper is open access
(MDPI, ACSAC, and similar), and never upload a paywalled PDF to the Claude
project or a shared drive.

---

## A. OpenPLC, Modbus TCP and ICS testbeds (closest to our stack)

| Paper | What it may help with | Status |
|---|---|---|
| [Development of an Open-Source Testbed Based on the … (title cut off in search)](https://ppee.unb.br/wp-content/uploads/2023/01/DEVELOPMENT.pdf) | Direct comparison for our open-source ICS testbed. Get the full title from the PDF. | To read |
| [Securing Modbus TCP Communications in I4.0: A Penetration Testing Approach Using OpenPLC and Factory IO](https://www.researchgate.net/publication/378724977_Securing_Modbus_TCP_Communications_in_I40_A_Penetration_Testing_Approach_Using_OpenPLC_and_Factory_IO) | Same OpenPLC + Modbus TCP stack, with attacks run against it. Attack-scenario ideas. | To read |
| [SCADA Testbed Implementation, Attacks, and Security Solutions](https://www.researchgate.net/publication/350826524_SCADA_Testbed_Implementation_Attacks_and_Security_Solutions) | Full build, attack and defense cycle for a SCADA testbed. | To read |
| [Virtualizing Industrial Control Systems Testbeds for Cybersecurity Research](https://www.acsac.org/2016/program/files/02-Virtualization%20of%20Industrial%20Control%20System%20Testbeds%20for%20Cybersecurity-Thiago%20Alves.pdf) | Appears to be by Thiago Alves (OpenPLC's author), which would support our OpenPLC choice. Confirm authorship. Open access (ACSAC). | To read |
| [Assessing the impact of Modbus/TCP protocol attacks on critical infrastructure: WWTP case study](https://www.sciencedirect.com/science/article/pii/S0045790625004288) | Impact of Modbus/TCP attacks; a recent paper. | To read |
| [Cyber-security Modbus ICS dataset (IEEE DataPort)](https://ieee-dataport.org/documents/cyber-security-modbus-ics-dataset) | A dataset, not a paper. Possible source of attack traffic for detection work. | To read |

## B. Education and testbed design

| Paper | What it may help with | Status |
|---|---|---|
| [Virtual Testbed for Cyber-Physical System Security Research and Education: Design, Evaluation, and Impact](https://doi.org/10.3390/electronics15030582) | Very close to our goal of an educational CPS testbed. MDPI, open access. | To read |
| [What Makes an Industrial Control System Security Testbed Credible and Acceptable? Towards a Design Consideration Framework](https://www.researchgate.net/publication/353398409_What_Makes_an_Industrial_Control_System_Security_Testbed_Credible_and_Acceptable_Towards_a_Design_Consideration_Framework) | Design criteria we could use to justify the two-building, 10-node architecture. | To read |
| [A Survey on Smart Grid Cyber-Physical System Testbeds](https://csl.fiu.edu/wp-content/uploads/2023/05/smart_grid_survey.pdf) | Background survey. The PDF header shows IEEE Communications Surveys & Tutorials, vol. 19, no. 1, 2017. | To read |
| [EPIC: An Electric Power Testbed for Research and Training in Cyber Physical Systems Security](https://www.researchgate.net/publication/330758995_EPIC_An_Electric_Power_Testbed_for_Research_and_Training_in_Cyber_Physical_Systems_Security_17th_International_Workshop_IWDW_2018_Jeju_Island_Korea_October_22-24_2018_Proceedings) | Well-known training testbed to compare against. IWDW 2018 proceedings (per the link text). | To read |

## C. Mechanical and manufacturing side (ASME)

| Paper | What it may help with | Status |
|---|---|---|
| [Cyber-Physical Security Vulnerabilities Identification and Classification in Smart Manufacturing](https://asmedigitalcollection.asme.org/computingengineering/article/25/9/091005/1218545/Cyber-Physical-Security-Vulnerabilities) | Vulnerability taxonomy for the ME side of the guide. ASME Journal of Computing and Information Science in Engineering (per the URL). | To read |
| [Cyber-Physical System Implementation for Manufacturing With Analytics in the Cloud Layer](https://asmedigitalcollection.asme.org/computingengineering/article/22/1/011011/1114318/Cyber-Physical-System-Implementation-for) | CPS architecture layers. Same ASME journal (per the URL). | To read |
| [Security in Cyber-Enabled Design and Manufacturing: A Survey](https://ebooks.asmedigitalcollection.asme.org/computingengineering/article-pdf/doi/10.1115/1.4040341/6365588/jcise_018_04_040802.pdf) | Survey. DOI 10.1115/1.4040341 (from the URL). | To read |

---

## Still to do

- [ ] Open each paper, copy the exact citation, and record it under the entry
- [ ] Update each status; drop anything that turns out to be off-topic
- [ ] Check which of these your university library gives full-text access to
      (IEEE Xplore, ASME Digital Collection, ScienceDirect)
- [ ] Decide the citation style your professor wants (IEEE, ASME or APA)
