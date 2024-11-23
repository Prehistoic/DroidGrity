import logging
import re
from typing import Dict
import traceback

class TemplateFiller:

    def __init__(self, template: str):
        self.logger = logging.getLogger(__name__)
        self.template = template

    # Expected field that needs to be filled in the template must follow this placeholder pattern :
    # @droidgrity.filler.KEY@
    # where KEY is the key used in the dictionary passed to the fill method
    def fill(self, data: Dict):
        try:
            assert self.template.split(".")[-1] == "template"
            output = self.template.replace(".template", "")

            self.logger.info(f"Reading template {self.template}...")
            with open(self.template, 'r') as f:
                content = f.read()

            pattern = r"@droidgrity\.filler\.(\w+)@"

            def replacer(match):
                key = match.group(1) # Extract the KEY from the placeholder
                replacement = data.get(key, match.group(0)) # Replace if key exists, else keep original
                self.logger.info(f"Found placeholder to fill : {key} => {replacement}")
                return replacement
            
            updated_content = re.sub(pattern, replacer, content)

            with open(output, 'w') as f:
                f.write(updated_content)
            
            return output
        except Exception:
            self.logger.error(f"Error when filling template:\n{traceback.format_exc()}")
            return None