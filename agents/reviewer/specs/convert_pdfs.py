#!/usr/bin/env python3
"""Convert PDF files in ./pdf to Markdown files in ./markdown."""

import re
import sys
from pathlib import Path

try:
    import fitz  # pymupdf
except ImportError:
    sys.exit("pymupdf not installed. Run: pip install pymupdf")

PDF_DIR = Path(__file__).parent / "pdf"
MD_DIR = Path(__file__).parent / "markdown"


def block_to_markdown(block: dict) -> str:
    """Convert a pymupdf text block to a markdown string."""
    lines = []
    for line in block.get("lines", []):
        spans = line.get("spans", [])
        if not spans:
            continue

        text = "".join(s["text"] for s in spans).strip()
        if not text:
            continue

        # Use font size of the dominant span to detect headings
        max_size = max(s["size"] for s in spans)
        flags = spans[0]["flags"]  # bold = bit 4 (16), italic = bit 1 (2)
        is_bold = bool(flags & 16)

        if max_size >= 18:
            lines.append(f"# {text}")
        elif max_size >= 14:
            lines.append(f"## {text}")
        elif max_size >= 12 and is_bold:
            lines.append(f"### {text}")
        else:
            lines.append(text)

    return "\n".join(lines)


def pdf_to_markdown(pdf_path: Path) -> str:
    doc = fitz.open(pdf_path)
    sections: list[str] = []

    for page_num, page in enumerate(doc, start=1):
        page_blocks: list[str] = [f"<!-- page {page_num} -->"]
        blocks = page.get_text("dict")["blocks"]

        for block in blocks:
            if block["type"] != 0:  # 0 = text, 1 = image
                continue
            md = block_to_markdown(block)
            if md.strip():
                page_blocks.append(md)

        sections.append("\n\n".join(page_blocks))

    doc.close()

    full_text = "\n\n---\n\n".join(sections)

    # Collapse runs of 3+ blank lines to 2
    full_text = re.sub(r"\n{3,}", "\n\n", full_text)
    return full_text


def main() -> None:
    if not PDF_DIR.is_dir():
        sys.exit(f"PDF directory not found: {PDF_DIR}")

    MD_DIR.mkdir(exist_ok=True)

    pdfs = sorted(PDF_DIR.glob("*.pdf"))
    if not pdfs:
        print("No PDF files found.")
        return

    for pdf_path in pdfs:
        out_path = MD_DIR / (pdf_path.stem + ".md")
        print(f"Converting: {pdf_path.name} ...", end=" ", flush=True)
        try:
            md = pdf_to_markdown(pdf_path)
            out_path.write_text(md, encoding="utf-8")
            print(f"-> {out_path.name}")
        except Exception as exc:
            print(f"FAILED: {exc}")


if __name__ == "__main__":
    main()
