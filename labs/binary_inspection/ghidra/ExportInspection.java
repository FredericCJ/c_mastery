// Export the analyst's function name and plate comment at the current cursor.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.program.model.listing.Function;
import ghidra.program.model.address.Address;
import com.google.gson.Gson;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.LinkedHashMap;

public class ExportInspection extends GhidraScript {
    public void run() throws Exception {
        String[] args = getScriptArgs();
        Address where = currentAddress;
        if (isRunningHeadless()) {
            where = currentProgram.getImageBase().add(Long.decode(args[1]));
        }
        Function function = getFunctionContaining(where);
        if (function == null) throw new IllegalArgumentException("Select a function body");
        DecompInterface decompiler = new DecompInterface();
        try {
            decompiler.openProgram(currentProgram);
            var result = decompiler.decompileFunction(function, 30, monitor);
            if (!result.decompileCompleted()) throw new IllegalStateException(result.getErrorMessage());
            var row = new LinkedHashMap<String, Object>();
            row.put("exe_sha256", currentProgram.getExecutableSHA256());
            row.put("rva", function.getEntryPoint().subtract(currentProgram.getImageBase()));
            row.put("name", function.getName());
            row.put("comment", function.getComment());
            row.put("decompiled", result.getDecompiledFunction().getC());
            Path output = isRunningHeadless() ? Path.of(args[0]) :
                askFile("Export annotation JSON", "Export").toPath();
            Files.writeString(output, new Gson().toJson(row));
        } finally {
            decompiler.dispose();
        }
    }
}
