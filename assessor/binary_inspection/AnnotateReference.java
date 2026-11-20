// Set the assessor reference annotation on the already-analyzed stripped image.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.SourceType;

public class AnnotateReference extends GhidraScript {
    public void run() throws Exception {
        var address = currentProgram.getImageBase().add(Long.decode(getScriptArgs()[0]));
        Function function = getFunctionAt(address);
        if (function == null) throw new IllegalStateException("No analyzed function at " + address);
        function.setName("inspect_weighted_record", SourceType.USER_DEFINED);
        function.setComment("Updates the call count and chooses the weighted or additive result by tag.");
    }
}
