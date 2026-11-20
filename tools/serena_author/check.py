"""Check the live read-only MCP tool surface and references in both pilot languages."""
import asyncio
from pathlib import Path
import sys
from tempfile import TemporaryFile

from mcp import ClientSession, StdioServerParameters, stdio_client

ROOT = Path(__file__).resolve().parents[2]


async def check(server_log):
    server = StdioServerParameters(command=sys.executable, args=[str(ROOT / "tools/author-tool"), "serena", "serve"])
    async with stdio_client(server, errlog=server_log) as (reader, writer):
        async with ClientSession(reader, writer, read_timeout_seconds=100) as session:
            await session.initialize()
            expected = {"initial_instructions", "get_symbols_overview",
                        "find_symbol", "find_declaration", "find_referencing_symbols"}
            actual = {tool.name for tool in (await session.list_tools()).tools}
            if actual != expected:
                raise RuntimeError(f"Unexpected Serena interface: {sorted(actual)}")
            for path, symbol, caller in [
                ("cmastery/cli.py", "locate", "instantiate"),
                ("tools/public_exports.py", "write_archive", "reproduce"),
                ("labs/foundations/src/foundation.c", "f_prefix", "labs/foundations/tests/tests.c"),
            ]:
                result = await session.call_tool("find_referencing_symbols", {"relative_path": path, "name_path": symbol})
                output = " ".join(item.text for item in result.content if item.type == "text")
                if result.is_error or caller not in output:
                    raise RuntimeError(f"Missing reference to {symbol} from {caller}: {output[:2000]}")
                print(f"{path}: {symbol} referenced by {caller}")
            for excluded in ("tools/check_author_tools.py", "assessor/foundations/foundation.c"):
                result = await session.call_tool("get_symbols_overview", {"relative_path": excluded})
                output = " ".join(item.text for item in result.content if item.type == "text")
                if not result.is_error or "Cannot extract symbols" not in output:
                    raise RuntimeError(f"Expected Serena to reject excluded path {excluded}")
            print("Five read-only tools available. Both out-of-scope symbol queries were rejected.")


if __name__ == "__main__":
    with TemporaryFile(mode="w+", encoding="utf-8") as log:
        try:
            asyncio.run(check(log))
        except Exception:
            log.seek(0)
            print(log.read()[-6000:], file=sys.stderr)
            raise
